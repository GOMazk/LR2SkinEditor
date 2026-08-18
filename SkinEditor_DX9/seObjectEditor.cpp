#include "seObjectEditor.h"
#include "winWorkspace.h"
#include "inputwrap.h"
#include "seHelper.h"
#include "op.h"
#include "resource.h"
#include <fstream>
#include <sstream>
#include <cstdlib>
#include <cstring>
#include <map>

SEObjectEditorModel g_seObjectEditorModel;

static std::string commandOf(SKINFILELINEREAD& r) {
    if (r.csv.str[0].body == NULL) return std::string();
    return std::string(r.csv.str[0].outstr());
}

static bool numericIndex(SKINFILELINEREAD& r, int& value) {
    if (r.csv.str[1].body == NULL || r.csv.str[1].length() == 0) return false;
    const char* s = r.csv.str[1].outstr();
    char* end = NULL;
    long v = std::strtol(s, &end, 10);
    if (end == s || *end != '\0') return false;
    value = (int)v;
    return true;
}

bool SEObjectEditorModel::LoadGroups(const char* path) {
    groups.clear();
    loadedPath = path ? path : "";
    std::string contents;
    if (path && *path) {
        std::ifstream source(path, std::ios::in | std::ios::binary);
        if (source) {
            std::ostringstream buffer;
            buffer << source.rdbuf();
            contents = buffer.str();
        }
    }
    if (contents.empty() && !LoadEmbeddedTextResource(IDR_SKIN_OBJ_GROUP_TXT, contents)) return false;

    std::istringstream f(contents);
    std::string line;
    SEObjectGroupDef* current = NULL;
    while (std::getline(f, line)) {
        if (!line.empty() && line.back() == '\r') line.pop_back();
        if (line.empty()) continue;
        if (line[0] == '$') {
            groups.push_back(SEObjectGroupDef());
            current = &groups.back();
            current->name = line.substr(1);
        } else if (line[0] == '#' && current) {
            std::size_t comma = line.find(',');
            current->commands.push_back(line.substr(0, comma));
        }
    }
    return !groups.empty();
}

const SEObjectGroupDef* SEObjectEditorModel::Group(int index) const {
    if (index < 0 || index >= (int)groups.size()) return NULL;
    return &groups[index];
}

std::vector<int> SEObjectEditorModel::ObjectsForGroup(int group) const {
    std::vector<int> out;
    for (int i = 0; i < (int)objects.size(); ++i)
        if (objects[i].group == group) out.push_back(i);
    return out;
}

std::vector<int> SEObjectEditorModel::ObjectsForUserGroup(int group) const {
    std::vector<int> out;
    if (group < 0 || group >= (int)userGroups.size()) return out;
    for (int i = 0; i < (int)objects.size(); ++i) {
        for (const std::string& id : userGroups[group].memberIds) {
            if (!id.empty() && objects[i].editorId == id) { out.push_back(i); break; }
        }
    }
    return out;
}

void SEObjectEditorModel::Rebuild(WORKSPACE& ws) {
    objects.clear();
    userGroups.clear();
    if (groups.empty()) return;

    // Build the command index once. Large HD skins can contain well over
    // 10,000 expanded CSV rows; scanning every row again for every object
    // group made opening such skins look like a crash.
    std::map<std::string, std::vector<int> > commandGroups;
    for (int gi = 0; gi < (int)groups.size(); ++gi) {
        const SEObjectGroupDef& def = groups[gi];
        for (std::size_t c = 0; c < def.commands.size(); ++c) {
            const std::string& cmd = def.commands[c];
            if (cmd == "#IF" || cmd == "#ELSEIF" || cmd == "#ELSE" || cmd == "#ENDIF") continue;
            std::vector<int>& owners = commandGroups[cmd];
            if (owners.empty() || owners.back() != gi) owners.push_back(gi);
        }
    }

    std::vector<std::vector<int> > matchingByGroup(groups.size());
    for (int r = 0; r < ws.skinfileLines.count; ++r) {
        SKINFILELINEREAD& line = ((SKINFILELINEREAD*)ws.skinfileLines.data)[r];
        std::map<std::string, std::vector<int> >::const_iterator found = commandGroups.find(commandOf(line));
        if (found == commandGroups.end()) continue;
        for (std::size_t i = 0; i < found->second.size(); ++i)
            matchingByGroup[found->second[i]].push_back(r);
    }

    for (int gi = 0; gi < (int)groups.size(); ++gi) {
        const SEObjectGroupDef& def = groups[gi];
        const std::vector<int>& matching = matchingByGroup[gi];
        if (matching.empty()) continue;

        // Only use CSV column 1 as an object key when skinHelper explicitly
        // calls that argument "index".  For IMAGE/NUMBER/etc. column 1 is
        // usually a graphic number or another value, not an object index.
        bool indexedGroup = false;
        int srcTypeCount = 0;
        int dstTypeCount = 0;
        for (std::size_t c = 0; c < def.commands.size(); ++c) {
            if (def.commands[c].size() >= 4 && def.commands[c].compare(0, 4, "#SRC") == 0) ++srcTypeCount;
            if (def.commands[c].size() >= 4 && def.commands[c].compare(0, 4, "#DST") == 0) ++dstTypeCount;
            CSTR help = GetCommandHelp(def.commands[c].c_str(), 1);
            if (help.body && _stricmp(help.outstr(), "index") == 0) indexedGroup = true;
        }
        // skinHelper has incomplete argument metadata for NOTE-family commands.
        // Multiple distinct SRC/DST command types are nevertheless a strong
        // indication that column 1 is the shared object index.
        if (!indexedGroup && (srcTypeCount > 1 || dstTypeCount > 1)) {
            for (std::size_t i = 0; i < matching.size(); ++i) {
                int dummy = 0;
                if (numericIndex(((SKINFILELINEREAD*)ws.skinfileLines.data)[matching[i]], dummy)) {
                    indexedGroup = true;
                    break;
                }
            }
        }

        if (indexedGroup) {
            struct Bucket { int ifgroup; int key; std::vector<int> rows; };
            std::vector<Bucket> buckets;
            std::map<std::pair<int, int>, int> bucketIndex;
            for (std::size_t i = 0; i < matching.size(); ++i) {
                int key = 0;
                int ifgroup = ((SKINFILELINEREAD*)ws.skinfileLines.data)[matching[i]].ifgroup;
                if (!numericIndex(((SKINFILELINEREAD*)ws.skinfileLines.data)[matching[i]], key)) {
                    if (buckets.empty() || buckets.back().ifgroup != ifgroup) {
                        Bucket b; b.ifgroup = ifgroup; b.key = -1; buckets.push_back(b);
                    }
                    buckets.back().rows.push_back(matching[i]);
                    continue;
                }
                int bi = -1;
                std::pair<int, int> bucketKey(ifgroup, key);
                std::map<std::pair<int, int>, int>::const_iterator found = bucketIndex.find(bucketKey);
                if (found != bucketIndex.end()) bi = found->second;
                if (bi < 0) {
                    Bucket b; b.ifgroup = ifgroup; b.key = key; buckets.push_back(b);
                    bi = (int)buckets.size() - 1;
                    bucketIndex[bucketKey] = bi;
                }
                buckets[bi].rows.push_back(matching[i]);
            }
            for (std::size_t b = 0; b < buckets.size(); ++b) {
                SEObjectInstance o;
                o.group = gi;
                o.ifgroup = buckets[b].ifgroup;
                o.rows = buckets[b].rows;
                objects.push_back(o);
            }
        } else {
            // No explicit index: every SRC starts an instance. DST rows are
            // attached to the current SRC; DST-only groups become one row each.
            int current = -1;
            bool hasSrc = false;
            for (std::size_t i = 0; i < matching.size(); ++i) {
                int row = matching[i];
                std::string cmd = commandOf(((SKINFILELINEREAD*)ws.skinfileLines.data)[row]);
                bool isSrc = cmd.size() >= 4 && cmd.compare(0, 4, "#SRC") == 0;
                if (isSrc || !hasSrc) {
                    SEObjectInstance o;
                    o.group = gi;
                    o.ifgroup = ((SKINFILELINEREAD*)ws.skinfileLines.data)[row].ifgroup;
                    objects.push_back(o);
                    current = (int)objects.size() - 1;
                    hasSrc = isSrc;
                }
                if (current >= 0) objects[current].rows.push_back(row);
            }
        }
    }

    // TEXT objects get a useful default name from the field described as $st
    // in skinHelper.txt. Other object names remain user-editable and blank.
    for (std::size_t oi = 0; oi < objects.size(); ++oi) {
        SEObjectInstance& object = objects[oi];
        for (std::size_t ri = 0; ri < object.rows.size() && object.name.empty(); ++ri) {
            const int row = object.rows[ri];
            if (row < 0 || row >= ws.skinfileLines.count) continue;
            SKINFILELINEREAD& line = ((SKINFILELINEREAD*)ws.skinfileLines.data)[row];
            const char* command = line.csv.str[0].body ? line.csv.str[0].outstr() : "";
            for (int col = 1; col < 30; ++col) {
                CSTR help = GetCommandHelp(command, col);
                if (!help.left(3).isSame("$st")) continue;
                const char* defaultName = textName(line.csv.val[col]);
                if (defaultName && *defaultName) object.name = defaultName;
                break;
            }
        }
    }

    for (SEObjectInstance& object : objects) {
        if (object.rows.empty()) continue;
        for (int row = object.rows.front() - 1; row >= 0; --row) {
            SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)ws.skinfileLines.data)[row];
            const char* text = meta.line.body ? meta.line.outstr() : "";
            if (strncmp(text, "$SE_OBJECT_ID,", 14) == 0) {
                object.editorId = text + 14;
                break;
            }
            if (*text == '#') break;
            if (*text && *text != '$' && strncmp(text, "//", 2) != 0) break;
        }
    }

    SEUserObjectGroup* currentUserGroup = NULL;
    for (int row = 0; row < ws.skinfileLines.count; ++row) {
        SKINFILELINEREAD& meta = ((SKINFILELINEREAD*)ws.skinfileLines.data)[row];
        const char* text = meta.line.body ? meta.line.outstr() : "";
        if (strncmp(text, "$SE_GROUP_BEGIN,", 16) == 0) {
            userGroups.push_back(SEUserObjectGroup());
            currentUserGroup = &userGroups.back();
            currentUserGroup->name = text + 16;
        } else if (strncmp(text, "$SE_GROUP_MEMBER,", 17) == 0 && currentUserGroup) {
            currentUserGroup->memberIds.push_back(text + 17);
        } else if (strcmp(text, "$SE_GROUP_END") == 0) {
            currentUserGroup = NULL;
        }
    }
}
