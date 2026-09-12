"""Lossless PNG registration for fresh native presets; no model or network calls."""
from __future__ import annotations

from pathlib import Path
import re

PAGE_SIZE = 2048
SPACING = 2
MAX_ASSETS = 128
MAX_PIXELS = 32 * 1024 * 1024
MAX_FILE_BYTES = 32 * 1024 * 1024
MAX_TOTAL_FILE_BYTES = 64 * 1024 * 1024
GENERATED = Path("LR2files") / "Theme" / "Generated"


class AssetError(ValueError):
    pass


def _integer(value, low, high, label):
    if type(value) is not int or not low <= value <= high:
        raise AssetError(f"{label} must be an integer in [{low}, {high}].")
    return value


def validate_assets(value):
    """Validate the recipe without opening files; return normalized, independent data."""
    if not isinstance(value, list) or len(value) > MAX_ASSETS:
        raise AssetError(f"assets must be an array of at most {MAX_ASSETS} PNG assets.")
    result, seen, bindings = [], set(), set()
    for item in value:
        if not isinstance(item, dict) or set(item) - {"id", "path", "bind", "image", "div_x", "div_y", "cycle"}:
            raise AssetError("Asset fields are id, path, bind, image, div_x, div_y and cycle.")
        asset_id = item.get("id")
        if not isinstance(asset_id, str) or not re.fullmatch(r"[A-Za-z0-9_-]{1,64}", asset_id) or asset_id in seen:
            raise AssetError("Asset IDs must be unique ASCII letters, digits, underscores or hyphens (1..64).")
        seen.add(asset_id)
        path = item.get("path")
        if not isinstance(path, str) or not path or len(path) > 4096 or any(c in path for c in "\0\r\n") or Path(path).suffix.lower() != ".png":
            raise AssetError(f"Asset {asset_id}.path must name a local PNG file.")
        normalized = {"id": asset_id, "path": path,
                      "div_x": _integer(item.get("div_x", 1), 1, PAGE_SIZE, f"{asset_id}.div_x"),
                      "div_y": _integer(item.get("div_y", 1), 1, PAGE_SIZE, f"{asset_id}.div_y"),
                      "cycle": _integer(item.get("cycle", 0), 0, 600000, f"{asset_id}.cycle"), "bind": []}
        targets = item.get("bind", [])
        if not isinstance(targets, list) or len(targets) > 512:
            raise AssetError(f"Asset {asset_id}.bind must be an array of at most 512 source bindings.")
        for target in targets:
            if not isinstance(target, dict) or set(target) - {"object", "command", "index"}:
                raise AssetError("Each asset binding contains object, command and optional index.")
            object_id, command = target.get("object"), target.get("command")
            if not isinstance(object_id, str) or not re.fullmatch(r"[A-Za-z0-9_-]{1,96}", object_id):
                raise AssetError(f"Asset {asset_id} binding needs a stable Object ID.")
            if not isinstance(command, str) or not re.fullmatch(r"#SRC_[A-Za-z0-9_]+", command, re.IGNORECASE):
                raise AssetError(f"Asset {asset_id} binding command must be a #SRC_ command.")
            normalized_target = {"object": object_id, "command": command.upper()}
            if "index" in target:
                normalized_target["index"] = _integer(target["index"], -32768, 32767, f"{asset_id}.bind.index")
            key = (object_id, command.upper(), normalized_target.get("index"))
            if key in bindings:
                raise AssetError(f"Repeated asset binding for {object_id}/{command}.")
            bindings.add(key)
            normalized["bind"].append(normalized_target)
        if "image" in item:
            placement = item["image"]
            if not isinstance(placement, dict) or set(placement) - {"x", "y", "w", "h", "layer"}:
                raise AssetError("image must contain x, y, w, h and optional layer.")
            normalized["image"] = {field: _integer(placement.get(field), *bounds, f"{asset_id}.image.{field}")
                                   for field, bounds in (("x", (-8192, 8192)), ("y", (-8192, 8192)),
                                                         ("w", (1, 8192)), ("h", (1, 8192)))}
            layer = placement.get("layer", "front")
            if layer not in ("back", "front"):
                raise AssetError("image.layer must be back or front.")
            normalized["image"]["layer"] = layer
        if not normalized["bind"] and "image" not in normalized:
            raise AssetError(f"Asset {asset_id} requires a nonempty bind array or an image placement.")
        result.append(normalized)
    return result


def _load_images(assets, recipe_dir):
    try:
        from PIL import Image
    except ImportError as error:
        raise AssetError("PNG assets require Pillow. Install tools/requirements-agent.txt first.") from error
    images, pixel_count, byte_count = [], 0, 0
    try:
        for asset in assets:
            source = Path(asset["path"])
            if not source.is_absolute():
                source = recipe_dir / source
            try:
                size = source.stat().st_size
                byte_count += size
                if size > MAX_FILE_BYTES or byte_count > MAX_TOTAL_FILE_BYTES:
                    raise AssetError("PNG input exceeds the 32 MiB/file or 64 MiB total compressed-file budget.")
                with Image.open(source) as candidate:
                    if candidate.format != "PNG" or getattr(candidate, "is_animated", False):
                        raise AssetError(f"Asset {asset['id']} must be a static PNG; use a sprite sheet for animation.")
                    width, height = candidate.size
                    pixel_count += width * height
                    if not 1 <= width <= PAGE_SIZE or not 1 <= height <= PAGE_SIZE:
                        raise AssetError(f"Asset {asset['id']} exceeds the {PAGE_SIZE}x{PAGE_SIZE} atlas page.")
                    if pixel_count > MAX_PIXELS:
                        raise AssetError("PNG inputs exceed the 32 megapixel decoded-pixel budget.")
                    if width % asset["div_x"] or height % asset["div_y"]:
                        raise AssetError(f"Asset {asset['id']} dimensions must divide evenly into div_x/div_y frames.")
                    candidate.verify()
                with Image.open(source) as candidate:
                    images.append(candidate.convert("RGBA"))
            except AssetError:
                raise
            except (OSError, ValueError, SyntaxError, Image.DecompressionBombError) as error:
                raise AssetError(f"Cannot read PNG asset {asset['id']}: {error}") from error
        return Image, images
    except BaseException:
        for image in images:
            image.close()
        raise


def _pack(images):
    """Stable shelf packing in recipe order, without scaling or rotating any pixels."""
    pages, placements = [], []
    x = y = row_height = 0
    for image in images:
        width, height = image.size
        if not pages:
            pages.append([0, 0])
        if x + width > PAGE_SIZE:
            x, y, row_height = 0, y + row_height + SPACING, 0
        if y + height > PAGE_SIZE:
            pages.append([0, 0])
            x = y = row_height = 0
        placements.append({"page": len(pages) - 1, "x": x, "y": y, "w": width, "h": height})
        pages[-1][0] = max(pages[-1][0], x + width)
        pages[-1][1] = max(pages[-1][1], y + height)
        x += width + SPACING
        row_height = max(row_height, height)
    return pages, placements


def _set_fields(tokens, values, columns, label):
    fields = columns.get(tokens[0], {})
    for field, value in values.items():
        column = fields.get(field)
        if type(column) is not int or not 1 <= column < len(tokens):
            raise AssetError(f"Native schema has no usable {field} column in {tokens[0]} ({label}).")
        tokens[column] = str(value)
    return ",".join(tokens)


def _new_row(command, values, columns, label):
    fields = columns.get(command, {})
    if not fields or any(type(column) is not int or not 1 <= column <= 64 for column in fields.values()):
        raise AssetError(f"Native schema has no usable columns for {command}.")
    return _set_fields([command] + ["0"] * max(fields.values()), values, columns, label)


def _bind_rows(lines, assets, placements, columns, initial_gr):
    rows, object_counts, current_id = {}, {}, None
    for number, line in enumerate(lines):
        body = line.rstrip("\r\n")
        if body.startswith("$SE_OBJECT_ID,"):
            current_id = body.split(",", 1)[1]
            object_counts[current_id] = object_counts.get(current_id, 0) + 1
        elif body.startswith("#SRC_"):
            tokens = body.split(",")
            rows.setdefault((current_id, tokens[0]), []).append((number, tokens))
    touched = set()
    for asset, rectangle in zip(assets, placements):
        source = {key: rectangle[key] for key in ("x", "y", "w", "h")}
        source.update(gr=initial_gr + rectangle["page"], div_x=asset["div_x"], div_y=asset["div_y"], cycle=asset["cycle"])
        for target in asset["bind"]:
            object_id, command = target["object"], target["command"]
            if object_counts.get(object_id) != 1:
                raise AssetError(f"Binding Object {object_id} is missing or has duplicate IDs.")
            candidates = rows.get((object_id, command), [])
            if "index" in target:
                column = columns.get(command, {}).get("index")
                if type(column) is not int or column < 1:
                    raise AssetError(f"Native schema has no index field for {command}; omit bind.index.")
                candidates = [(number, tokens) for number, tokens in candidates
                              if column < len(tokens) and tokens[column].strip() == str(target["index"])]
            if len(candidates) != 1:
                raise AssetError(f"Binding {object_id}/{command} matches {len(candidates)} rows; select one using index.")
            number, tokens = candidates[0]
            if number in touched:
                raise AssetError(f"More than one asset binding targets the same {object_id}/{command} row.")
            touched.add(number)
            body = lines[number].rstrip("\r\n")
            lines[number] = _set_fields(tokens, source, columns, asset["id"]) + lines[number][len(body):]
    return set(object_counts)


def apply_assets(text, assets, recipe_dir: Path, output_dir: Path, columns: dict):
    """Return patched preset text and atlas manifest; write only fresh atlas PNGs.

    The caller owns a new build staging directory. Existing skins, source PNGs and
    their encodings are never written. gr numbers count the original #IMAGE rows.
    """
    assets = validate_assets(assets)
    empty = {"atlases": [], "assets": [], "added_object_ids": []}
    if not assets:
        return text, empty
    lines = text.splitlines(keepends=True)
    for line in lines:
        if line.split(",", 1)[0].strip().upper() in {"#INCLUDE", "#IF", "#ELSEIF", "#ELSE", "#ENDIF"}:
            raise AssetError("PNG registration supports the fresh single-file preset, not include/conditional skin edits.")
    image_rows = [i for i, line in enumerate(lines) if line.startswith("#IMAGE,")]
    object_rows = [i for i, line in enumerate(lines) if line.startswith(("$SE_OBJECT_NAME,", "$SE_OBJECT_ID,", "#SRC_"))]
    if not image_rows or not object_rows or max(image_rows) >= min(object_rows):
        raise AssetError("Expected native preset #IMAGE declarations before all Objects.")
    folder = Path(output_dir) / GENERATED
    if not folder.resolve().is_relative_to(Path(output_dir).resolve()) or not folder.is_dir():
        raise AssetError("The native Generated output folder is missing or outside the build stage.")
    Image, images = _load_images(assets, Path(recipe_dir))
    written = []
    try:
        pages, placements = _pack(images)
        atlas_entries = [{"path": (GENERATED / f"agent-atlas-{i:03d}.png").as_posix(),
                          "gr": len(image_rows) + i, "width": size[0], "height": size[1]}
                         for i, size in enumerate(pages)]
        for entry in atlas_entries:
            target = Path(output_dir) / entry["path"]
            if target.exists() or target.is_symlink():
                raise AssetError(f"Atlas output already exists: {entry['path']}; it was not replaced.")
        known_ids = _bind_rows(lines, assets, placements, columns, len(image_rows))
        newline = "\r\n" if "\r\n" in text else "\n"
        back, front, manifest_assets, added_ids = [], [], [], []
        for asset, rectangle in zip(assets, placements):
            entry = {"id": asset["id"], "atlas": atlas_entries[rectangle["page"]]["path"],
                     "gr": len(image_rows) + rectangle["page"],
                     **{key: rectangle[key] for key in ("x", "y", "w", "h")},
                     **{key: asset[key] for key in ("div_x", "div_y", "cycle")}, "bindings": asset["bind"]}
            if "image" in asset:
                object_id = "asset_" + asset["id"]
                if object_id in known_ids:
                    raise AssetError(f"Generated image Object ID {object_id} already exists.")
                known_ids.add(object_id)
                added_ids.append(object_id)
                entry["object_id"] = object_id
                entry["image"] = dict(asset["image"])
                source = {key: entry[key] for key in ("gr", "x", "y", "w", "h", "div_x", "div_y", "cycle")}
                destination = {key: asset["image"][key] for key in ("x", "y", "w", "h")}
                # Imported PNG alpha needs normal alpha blending, including pixels
                # whose preserved RGB bytes are nonzero while alpha is zero.
                destination.update(a=255, r=255, g=255, b=255, blend=1)
                block = ["", "$SE_OBJECT_NAME,Asset " + asset["id"], "$SE_OBJECT_ID," + object_id,
                         _new_row("#SRC_IMAGE", source, columns, object_id),
                         _new_row("#DST_IMAGE", destination, columns, object_id), ""]
                (back if asset["image"]["layer"] == "back" else front).append(newline.join(block))
            manifest_assets.append(entry)
        declarations = "".join("#IMAGE," + entry["path"].replace("/", "\\") + newline for entry in atlas_entries)
        # Insert after the last original declaration so original gr indices survive.
        lines[max(image_rows)] = lines[max(image_rows)].rstrip("\r\n") + newline + declarations
        lines[min(object_rows)] = "".join(back) + lines[min(object_rows)]
        result = "".join(lines)
        if front:
            result = result.rstrip("\r\n") + newline + "".join(front)
        try:
            result.encode("cp932", errors="strict")
        except UnicodeEncodeError as error:
            raise AssetError("Preset text cannot be preserved losslessly in CP932.") from error
        for page, entry in enumerate(atlas_entries):
            with Image.new("RGBA", (entry["width"], entry["height"]), (0, 0, 0, 0)) as atlas:
                for source, rectangle in zip(images, placements):
                    if rectangle["page"] == page:
                        # No alpha mask: even RGB bytes under fully transparent pixels survive.
                        atlas.paste(source, (rectangle["x"], rectangle["y"]))
                target = Path(output_dir) / entry["path"]
                with target.open("xb") as stream:
                    written.append(target)
                    atlas.save(stream, format="PNG")
        return result, {"atlases": atlas_entries, "assets": manifest_assets, "added_object_ids": added_ids}
    except BaseException as error:
        # These paths were exclusively created by this invocation. Leave no partial
        # atlas set if a later page fails; the caller also discards its build stage.
        for target in written:
            target.unlink(missing_ok=True)
        if isinstance(error, OSError):
            raise AssetError(f"Could not write fresh PNG atlas: {error}") from error
        raise
    finally:
        for image in images:
            image.close()
