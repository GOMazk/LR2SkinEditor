#include "selfTests.h"

#include "../LR2/En_fileutil.h"
#include "../LR2/En_timer.h"
#include "../LR2/LR2_skinmanage.h"
#include "olrSkin.h"
#include "seHelper.h"
#include "seObjectEditor.h"
#include "skin.h"
#include "skinBrowser.h"
#include "skinPathResolver.h"
#include "skinResolution.h"
#include "uiCatalog.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <iterator>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace {

bool IsSchemaField(const char* command, int column, const char* expected) {
    CSTR field = GetCommandHelp(command, column);
    field.trimWhiteSpace();
    return field.body && std::strcmp(field.outstr(), expected) == 0;
}

bool GroupContains(const SEObjectGroupDef& group, const char* command) {
    for (const std::string& candidate : group.commands) {
        if (candidate == command) return true;
    }
    return false;
}

const SEObjectGroupDef* FindGroup(const SEObjectEditorModel& model,
    const char* name) {
    const std::vector<SEObjectGroupDef>& groups = model.Groups();
    for (const SEObjectGroupDef& group : groups) {
        if (group.name == name) return &group;
    }
    return nullptr;
}

}

int RunSchemaContractSelfTest() {
    // Passing no override path exercises the RCDATA used by packaged builds.
    if (LoadCommandHelp(nullptr) != 0) return 1;
    if (arr_CommandHelp.count != 118) return 2;

    if (!IsSchemaField("#SRC_NUMBER", 11, "$num")) return 3;
    if (!IsSchemaField("#SRC_SLIDER", 13, "$type")) return 4;
    if (!IsSchemaField("#SRC_BUTTON", 11, "$type")) return 5;
    if (!IsSchemaField("#SRC_BARGRAPH", 11, "$type")) return 6;
    if (!IsSchemaField("#SRC_BGA", 1, "unused")) return 21;
    // M.H uses BARGRAPH destinations to gate its 1P/2P layout. A missing
    // leading NULL column or duplicated op1 shifts timer/OP conditions and
    // makes most of the skin disappear after an OLR round trip.
    if (!IsSchemaField("#DST_BARGRAPH", 1, "(NULL)") ||
        !IsSchemaField("#DST_BARGRAPH", 2, "time") ||
        !IsSchemaField("#DST_BARGRAPH", 16, "loop") ||
        !IsSchemaField("#DST_BARGRAPH", 17, "$timer") ||
        !IsSchemaField("#DST_BARGRAPH", 18, "$op1") ||
        !IsSchemaField("#DST_BARGRAPH", 19, "$op2") ||
        !IsSchemaField("#DST_BARGRAPH", 20, "$op3"))
        return 27;

    if (GetCommandValueKind("#SRC_NUMBER", "$num") != SE_VALUE_NUMBER)
        return 7;
    if (GetCommandValueKind("#SRC_SLIDER", "$type") != SE_VALUE_SLIDER)
        return 8;
    if (GetCommandValueKind("#SRC_BUTTON", "$type") != SE_VALUE_BUTTON)
        return 9;
    if (GetCommandValueKind("#SRC_BARGRAPH", "$type") != SE_VALUE_BARGRAPH)
        return 10;

    // Keep every symbolic combo range in sync with the highest id currently
    // declared by op.cpp. These providers are shared by CSV Table, New Object
    // and Object Inspector, so one truncated range hides the value everywhere.
    if (GetCommandValueItemCount(SE_VALUE_TEXT) <= 302 ||
        std::strcmp(GetCommandValueName(SE_VALUE_TEXT, 302), "LIFT_ONOFF") != 0)
        return 22;
    if (GetCommandValueItemCount(SE_VALUE_NUMBER) <= 422 ||
        std::strcmp(GetCommandValueName(SE_VALUE_NUMBER, 422), "LIFT_2P") != 0)
        return 23;
    if (GetCommandValueItemCount(SE_VALUE_BUTTON) <= 401 ||
        std::strcmp(GetCommandValueName(SE_VALUE_BUTTON, 401), "stretch") != 0)
        return 24;
    if (GetCommandValueItemCount(SE_VALUE_SLIDER) <= 28 ||
        std::strcmp(GetCommandValueName(SE_VALUE_SLIDER, 28), "LIFT2P") != 0)
        return 25;
    if (GetCommandValueItemCount(SE_VALUE_BARGRAPH) <= 59 ||
        std::strcmp(GetCommandValueName(SE_VALUE_BARGRAPH, 59), "2P_ratio_slow") != 0)
        return 26;

    SEObjectEditorModel model;
    if (!model.LoadGroups(nullptr)) return 11;
    if (model.Groups().size() != 40) return 12;

    const SEObjectGroupDef* number = FindGroup(model, "NUMBER");
    if (!number || !GroupContains(*number, "#SRC_NUMBER") ||
        !GroupContains(*number, "#DST_NUMBER"))
        return 13;

    const SEObjectGroupDef* note = FindGroup(model, "NOTE");
    if (!note || !GroupContains(*note, "#SRC_NOTE") ||
        !GroupContains(*note, "#DST_NOTE"))
        return 14;

    const SEObjectGroupDef* barTitle = FindGroup(model, "BAR_TITLE");
    if (!barTitle || !GroupContains(*barTitle, "#SRC_BAR_TITLE") ||
        !GroupContains(*barTitle, "#DST_BAR_TITLE"))
        return 20;

    std::vector<SEObjectInstance> rowObjects(2);
    rowObjects[0].rows.push_back(3);
    rowObjects[0].rows.push_back(5);
    rowObjects[1].rows.push_back(8);
    if (SEFindObjectForRow(rowObjects, 3) != 0) return 15;
    if (SEFindObjectForRow(rowObjects, 8) != 1) return 16;
    if (SEFindObjectForRow(rowObjects, 4) != -1) return 17;

    int dstWithArgb = 0;
    int dstWithoutArguments = 0;
    for (int commandIndex = 0; commandIndex < arr_CommandHelp.count;
        ++commandIndex) {
        CSVbuf& schema = ((CSVbuf*)arr_CommandHelp.data)[commandIndex];
        const char* command = schema.str[0].body ? schema.str[0].outstr() : "";
        if (std::strncmp(command, "#DST", 4) != 0) continue;
        if (!schema.str[1].body || !*schema.str[1].outstr()) {
            ++dstWithoutArguments;
            continue;
        }

        int argb[4] = { -1, -1, -1, -1 };
        const char* names[4] = { "a", "r", "g", "b" };
        for (int column = 1; column < 30; ++column) {
            CSTR field(schema.str[column]);
            field.trimWhiteSpace();
            const char* label = field.body ? field.outstr() : "";
            for (int component = 0; component < 4; ++component)
                if (std::strcmp(label, names[component]) == 0)
                    argb[component] = column;
        }
        if (argb[0] < 0 || argb[1] != argb[0] + 1 ||
            argb[2] != argb[0] + 2 || argb[3] != argb[0] + 3)
            return 18;
        ++dstWithArgb;
    }
    if (dstWithArgb != 34 || dstWithoutArguments != 3) return 19;

    return 0;
}

int RunResolutionEstimatorSelfTest() {
    SESkinResolutionDecision decision = SEResolveSkinResolution({
        "#INFORMATION,0,title,maker,,,1920,1080",
        "#RESOLUTION,1280,720",
        "#DST_IMAGE,0,0,0,0,3840,2160"
    });
    if (decision.width != 1920 || decision.height != 1080 ||
        decision.source != SESkinResolutionSource::Information)
        return 1;

    decision = SEResolveSkinResolution({
        "#INFORMATION,0,title,maker,,,,",
        "#RESOLUTION,1280,720",
        "#DST_IMAGE,0,0,0,0,1920,1080"
    });
    if (decision.width != 1280 || decision.height != 720 ||
        decision.source != SESkinResolutionSource::ResolutionCommand)
        return 2;

    decision = SEResolveSkinResolution({ "#RESOLUTION,1" });
    if (decision.width != 1280 || decision.height != 720 ||
        decision.source != SESkinResolutionSource::ResolutionCommand)
        return 3;

    // TenRiff regression: lanes left of x=960 look SD by themselves, but the
    // corner-anchored 1280x720 backdrop identifies the HD authoring canvas.
    decision = SEResolveSkinResolution({
        "#DST_IMAGE,0,0,0,0,1280,720",
        "#DST_NOTE,0,0,200,500,60,33,0,255",
        "#DST_NOTE,1,0,260,500,60,33,0,255"
    });
    if (decision.width != 1280 || decision.height != 720 ||
        decision.source != SESkinResolutionSource::TenRiffAuto ||
        decision.destinationEvidenceCount != 3)
        return 4;

    // TenRiff regression: large animation panels parked away from the origin
    // must not promote an ordinary SD playfield to HD or FHD.
    decision = SEResolveSkinResolution({
        "#DST_IMAGE,0,0,0,1335,1280,364",
        "#DST_IMAGE,1,0,0,-341,1280,341",
        "#DST_NOTE,0,0,100,400,30,22,0,255",
        "#DST_NOTE,1,0,130,400,30,22,0,255"
    });
    if (decision.width != 640 || decision.height != 480 ||
        decision.source != SESkinResolutionSource::TenRiffAuto ||
        decision.destinationEvidenceCount != 2)
        return 5;

    decision = SEResolveSkinResolution({
        "#DST_NOTE,0,0,1200,500,60,33,0,255"
    });
    if (decision.width != 1280 || decision.height != 720 ||
        decision.source != SESkinResolutionSource::TenRiffAuto)
        return 6;

    // TenRiff recognises LR2's three authoring families; even a larger
    // backdrop resolves to the highest supported FHD family.
    decision = SEResolveSkinResolution({
        "#DST_IMAGE,0,0,0,0,3840,2160"
    });
    if (decision.width != 1920 || decision.height != 1080 ||
        decision.source != SESkinResolutionSource::TenRiffAuto)
        return 7;

    // The final alpha-zero destination removes an otherwise off-screen lane,
    // matching TenRiff's visible-lane map rather than accumulating keyframes.
    decision = SEResolveSkinResolution({
        "#DST_NOTE,0,0,1700,500,60,33,0,255",
        "#DST_NOTE,0,100,1700,500,60,33,0,0",
        "#DST_NOTE,1,0,100,400,30,22,0,255"
    });
    if (decision.width != 640 || decision.height != 480 ||
        decision.source != SESkinResolutionSource::TenRiffAuto ||
        decision.destinationEvidenceCount != 1)
        return 8;

    decision = SEResolveSkinResolution({ "#IMAGE,background.png" });
    if (decision.width != 640 || decision.height != 480 ||
        decision.source != SESkinResolutionSource::Default640x480 ||
        decision.destinationEvidenceCount != 0)
        return 9;

    std::string preparedScript;
    std::string resolutionError;
    if (!SEPrepareLr2ExportResolution(
        "#INFORMATION,0,title,maker,,,,\r\n"
        "#RESOLUTION,1280,720\r\n"
        "#IMAGE,background.png\r\n",
        1280, 720, preparedScript, resolutionError) ||
        preparedScript.find(
            "#INFORMATION,0,title,maker,,,1280,720\r\n") != 0 ||
        preparedScript.find(
            "$OLR_IGNORED_RESOLUTION,#RESOLUTION,1280,720\r\n") ==
                std::string::npos ||
        preparedScript.find("\r\n#RESOLUTION,") != std::string::npos)
        return 10;

    preparedScript = "must remain transactional";
    if (SEPrepareLr2ExportResolution("#RESOLUTION,1\r\n",
        1280, 720, preparedScript, resolutionError) ||
        !preparedScript.empty())
        return 11;

    return 0;
}

int RunOlrPackageSelfTest() {
    const int fileScopeResult = RunOlrFileScopeSelfTest();
    if (fileScopeResult != 0) return 100 + fileScopeResult;

    char temporaryRoot[MAX_PATH] = {};
    char testRoot[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, temporaryRoot)) return 1;
    if (!GetTempFileNameA(temporaryRoot, "OLR", 0, testRoot)) return 2;
    DeleteFileA(testRoot);
    if (!CreateDirectoryA(testRoot, NULL)) return 3;

    const std::string root = testRoot;
    const std::string virtualRoot = root + "\\Test";
    const std::string virtualNoteFolder = virtualRoot + "\\note";
    const std::string assetPath = virtualNoteFolder + "\\blue.png";
    const std::string previousPackagePath = virtualRoot + "\\previous.olrskin";
    const std::string packagePath = root + "\\test.olrskin";
    const std::string orderingPackagePath = root + "\\ordering.olrskin";
    const std::string preservationPackagePath = root + "\\preservation.olrskin";
    const std::string preservationExtractedPath = root + "\\preservation-imported";
    const std::string preservationMaterializedPath = root + "\\preservation-materialized";
    const std::string preservationEditedMaterializedPath =
        root + "\\preservation-edited-materialized";
    const std::string tamperedPath = root + "\\tampered.olrskin";
    const std::string extractedPath = root + "\\imported";
    const std::string materializedPath = root + "\\materialized";
    const unsigned char assetBytes[] = {
        'O', 'L', 'R', '_', 'A', 'S', 'S', 'E', 'T', '_', 'T', 'E', 'S', 'T', '!'
    };
    int result = 0;
    if (!CreateDirectoryA(virtualRoot.c_str(), NULL) ||
        !CreateDirectoryA(virtualNoteFolder.c_str(), NULL))
        result = 4;
    if (result == 0) {
        FILE* assetFile = fopen(assetPath.c_str(), "wb");
        if (!assetFile) result = 5;
        else {
            if (fwrite(assetBytes, 1, sizeof(assetBytes), assetFile) !=
                sizeof(assetBytes))
                result = 6;
            fclose(assetFile);
        }
    }
    if (result == 0) {
        FILE* previousPackage = fopen(previousPackagePath.c_str(), "wb");
        if (!previousPackage) result = 36;
        else {
            fputs("nested package must not be copied", previousPackage);
            fclose(previousPackage);
        }
    }

    SEOLRSkinDocument document;
    document.title = "OLR self test";
    document.maker = "SkinEditor";
    document.scene = "PLAY 7KEYS";
    document.canvasWidth = 1280;
    document.canvasHeight = 720;
    document.resolutionSource = "#RESOLUTION";
    document.lr2Script =
        "#INFORMATION,0,OLR self test,SkinEditor,.\\VFS\\lr2files\\Theme\\Test\\note\\blue.png,,1280,720\r\n"
        "#RESOLUTION,1280,720\r\n"
        "#CUSTOMFILE,NOTE,.\\vFs\\LR2FILES\\Theme\\Test\\note\\*.png,blue\r\n"
        "#IMAGE,VFS/Lr2Files/Theme/Test/note/blue.png\r\n"
        "#IMAGE,assets/simple-note.png\r\n"
        "#SRC_IMAGE,0,0,0,0,16,16,1,1,,0\r\n"
        "#SRC_BUTTON,0,0,0,0,16,16,1,1,0,0,42,0,0\r\n"
        "#DST_IMAGE,0,0,100,200,16,16,0,255,255,255,255,1,0,0,0,800,46,119,948,0\r\n"
        "#DST_IMAGE,0,100,110,210,20,18,0,0,255,255,255,1,0,15,0,800,46,119,948,0\r\n"
        "#DST_IMAGE,1,0,50,60,10,12,0,200,255,255,255,0,0,0,0,,,,,\r\n"
        "#SRC_IMAGE,0,1,0,0,16,16,1,1,0,0\r\n"
        "#DST_IMAGE,0,25,400,300,80,40,0,128,255,255,255,2,0,10,0,222,33,6,7,8\r\n"
        "#DST_IMAGE,0,75,410,305,80,40,0,64,255,255,255,2,0,15,0,222,33,6,7,8\r\n";
    document.lr2ExportMainPath = "LR2files/Theme/Test/play.lr2skin";
    document.virtualRoots.push_back({ "LR2files/Theme/Test", virtualRoot });
    SEOLRSemanticObject semanticObject;
    semanticObject.id = "obj_test";
    semanticObject.category = "misc";
    semanticObject.name = "Test image";
    semanticObject.group = "IMAGE";
    SEOLRSemanticObject::Part semanticPart;
    semanticPart.id = "part_1";
    SEOLRSemanticObject::SourceBinding semanticSource;
    semanticSource.sourceRow = 6;
    semanticSource.sourceCommand = "#SRC_IMAGE";
    semanticPart.sources.push_back(semanticSource);
    SEOLRSemanticObject::SourceBinding buttonSemanticSource;
    buttonSemanticSource.sourceRow = 7;
    buttonSemanticSource.sourceCommand = "#SRC_BUTTON";
    semanticPart.sources.push_back(buttonSemanticSource);
    SEOLRSemanticObject::Destination semanticDestination;
    semanticDestination.id = "destination_1";
    semanticDestination.destinationCommand = "#DST_IMAGE";
    semanticDestination.layout = { 100, 200, 16, 16, 0, 1 };
    SEOLRSemanticObject::AnimationFrame firstFrame;
    firstFrame.destinationRow = 8;
    firstFrame.timeMs = 0;
    firstFrame.alpha = 255;
    firstFrame.transform = semanticDestination.layout;
    semanticDestination.animationFrames.push_back(firstFrame);
    SEOLRSemanticObject::AnimationFrame secondFrame;
    secondFrame.destinationRow = 9;
    secondFrame.timeMs = 100;
    secondFrame.alpha = 0;
    secondFrame.transform = { 110, 210, 20, 18, 15, 1 };
    semanticDestination.animationFrames.push_back(secondFrame);
    semanticDestination.hasTimer = true;
    semanticDestination.timer = 46;
    semanticDestination.hasLoop = true;
    semanticDestination.loop = 800;
    semanticDestination.hasOptions[0] = true;
    semanticDestination.options[0] = 119;
    semanticDestination.hasOptions[1] = true;
    semanticDestination.options[1] = 948;
    semanticPart.destinations.push_back(std::move(semanticDestination));
    SEOLRSemanticObject::Destination alternateSemanticDestination;
    alternateSemanticDestination.id = "destination_2";
    alternateSemanticDestination.destinationCommand = "#DST_IMAGE";
    alternateSemanticDestination.layout = { 50, 60, 10, 12, 0, 0 };
    SEOLRSemanticObject::AnimationFrame alternateFrame;
    alternateFrame.destinationRow = 10;
    alternateFrame.timeMs = 0;
    alternateFrame.alpha = 200;
    alternateFrame.transform = alternateSemanticDestination.layout;
    alternateSemanticDestination.animationFrames.push_back(alternateFrame);
    semanticPart.destinations.push_back(std::move(alternateSemanticDestination));
    semanticObject.parts.push_back(std::move(semanticPart));
    SEOLRSemanticObject::Part secondSemanticPart;
    secondSemanticPart.id = "part_2";
    SEOLRSemanticObject::SourceBinding secondSemanticSource;
    secondSemanticSource.sourceRow = 11;
    secondSemanticSource.sourceCommand = "#SRC_IMAGE";
    secondSemanticPart.sources.push_back(secondSemanticSource);
    SEOLRSemanticObject::Destination secondSemanticDestination;
    secondSemanticDestination.id = "destination_3";
    secondSemanticDestination.destinationCommand = "#DST_IMAGE";
    secondSemanticDestination.layout = { 400, 300, 80, 40, 10, 2 };
    SEOLRSemanticObject::AnimationFrame thirdFrame;
    thirdFrame.destinationRow = 12;
    thirdFrame.timeMs = 25;
    thirdFrame.alpha = 128;
    thirdFrame.transform = secondSemanticDestination.layout;
    secondSemanticDestination.animationFrames.push_back(thirdFrame);
    SEOLRSemanticObject::AnimationFrame fourthFrame;
    fourthFrame.destinationRow = 13;
    fourthFrame.timeMs = 75;
    fourthFrame.alpha = 64;
    fourthFrame.transform = { 410, 305, 80, 40, 15, 2 };
    secondSemanticDestination.animationFrames.push_back(fourthFrame);
    secondSemanticPart.destinations.push_back(std::move(secondSemanticDestination));
    semanticObject.parts.push_back(std::move(secondSemanticPart));
    document.objects.push_back(semanticObject);
    SEOLRSemanticObject compatibilityOnlyObject;
    compatibilityOnlyObject.id = "raw_only";
    compatibilityOnlyObject.category = "misc";
    compatibilityOnlyObject.name = "Unsupported destination";
    document.objects.push_back(compatibilityOnlyObject);
    SEOLRSimpleSlot simpleSlot;
    simpleSlot.id = "obj_test:#SRC_IMAGE:0";
    simpleSlot.category = "gear";
    simpleSlot.label = "Test image - Gear line";
    simpleSlot.objectId = "obj_test";
    simpleSlot.sourceCommand = "#SRC_IMAGE";
    simpleSlot.sourceRow = 6;
    simpleSlot.graphicId = 0;
    simpleSlot.width = 16;
    simpleSlot.height = 16;
    if (result == 0 && !SEIsOLRSimpleSlotCompilable(simpleSlot)) result = 40;
    SEOLRSimpleSlot legacyCropSlot = simpleSlot;
    legacyCropSlot.width = -1;
    legacyCropSlot.height = -1;
    if (result == 0 && SEIsOLRSimpleSlotCompilable(legacyCropSlot)) result = 41;
    document.simpleSlots.push_back(simpleSlot);
    document.assets.push_back({ 2, assetPath, "lr2/assets/simple-note.png" });
    document.sourceMap.push_back({ 0, 0, "main.lr2skin" });

    SEOLRPackageInfo packageInfo;
    std::string errorMessage;
    if (result == 0 && !SEWriteOLRSkinPackage(packagePath.c_str(), document,
        packageInfo, errorMessage)) result = 7;
    if (result == 0 && (packageInfo.formatVersion != 9 ||
        packageInfo.entries.size() != 8 ||
        packageInfo.objectCount != 1 || packageInfo.simpleSlotCount != 1 ||
        packageInfo.semanticPartCount != 2 || packageInfo.destinationCount != 3 ||
        packageInfo.assetCount != 2 ||
        packageInfo.virtualRootCount != 1 || packageInfo.virtualFileCount != 1 ||
        packageInfo.skippedVirtualFileCount != 1))
        result = 8;

    std::string extractedMain;
    if (result == 0 && !SEExtractOLRSkinPackage(packagePath.c_str(),
        extractedPath.c_str(), extractedMain, packageInfo, errorMessage))
        result = 9;
    if (result == 0 && packageInfo.compiledSimpleSlotCount != 1)
        result = 24;
    if (result == 0 && (packageInfo.compiledSemanticObjectCount != 1 ||
        packageInfo.compiledAnimationFrameCount != 5))
        result = 29;
    if (result == 0 && extractedMain != extractedPath + "\\main.lr2skin")
        result = 10;
    if (result == 0) {
        std::ifstream script(extractedMain, std::ios::binary);
        const std::string scriptBytes((std::istreambuf_iterator<char>(script)),
            std::istreambuf_iterator<char>());
        std::string expectedScript;
        std::string prepareError;
        if (!SEPrepareLr2ExportResolution(document.lr2Script, 1280, 720,
            expectedScript, prepareError) || scriptBytes != expectedScript ||
            scriptBytes.find("\r\n#RESOLUTION,") != std::string::npos)
            result = 11;
    }
    if (result == 0) {
        // The archive uses stored ZIP entries, so an unsupported Object id
        // would be visible here if skin.json or sections leaked a dangling
        // semantic reference. Its LR2 rows remain in the compatibility script.
        std::ifstream package(packagePath, std::ios::binary);
        const std::string packageBytes(
            (std::istreambuf_iterator<char>(package)),
            std::istreambuf_iterator<char>());
        if (packageBytes.find("raw_only") != std::string::npos)
            result = 35;
        if (result == 0 &&
            packageBytes.find("previous.olrskin") != std::string::npos)
            result = 37;
        // OLRskin is a user-approved 0.9 format lock. Pin both serialized
        // integer versions and the matching profile/authority identities.
        const std::string lockedVersion = "\"version\": 9";
        const size_t firstLockedVersion = packageBytes.find(lockedVersion);
        const size_t secondLockedVersion = firstLockedVersion ==
            std::string::npos ? std::string::npos : packageBytes.find(
                lockedVersion, firstLockedVersion + lockedVersion.size());
        if (result == 0 && (secondLockedVersion == std::string::npos ||
            packageBytes.find(
                "\"profile\": \"lr2-semantic-v0.9\"") ==
                    std::string::npos ||
            packageBytes.find(
                "\"authority\": \"lr2-destination-parts-v0.9\"") ==
                    std::string::npos))
            result = 109;
    }

    if (result == 0) {
        // The editor model groups Objects by command family, so its vector is
        // not LR2 draw order. The portable semantic index must nevertheless
        // expose Objects in the order of their first compiler-addressed row.
        SEOLRSkinDocument orderingDocument = document;
        orderingDocument.objects.clear();
        orderingDocument.simpleSlots.clear();
        SEOLRSemanticObject earlyObject = semanticObject;
        earlyObject.id = "early_object";
        earlyObject.parts.resize(1);
        SEOLRSemanticObject lateObject = semanticObject;
        lateObject.id = "late_object";
        lateObject.parts.erase(lateObject.parts.begin());
        orderingDocument.objects.push_back(std::move(lateObject));
        orderingDocument.objects.push_back(std::move(earlyObject));

        SEOLRPackageInfo orderingInfo;
        if (!SEWriteOLRSkinPackage(orderingPackagePath.c_str(),
            orderingDocument, orderingInfo, errorMessage))
            result = 80;
        if (result == 0 && orderingInfo.objectCount != 2)
            result = 81;
        if (result == 0) {
            std::ifstream orderingPackage(orderingPackagePath,
                std::ios::binary);
            const std::string orderingBytes(
                (std::istreambuf_iterator<char>(orderingPackage)),
                std::istreambuf_iterator<char>());
            const size_t earlyAt = orderingBytes.find(
                "\"id\": \"early_object\"");
            const size_t lateAt = orderingBytes.find(
                "\"id\": \"late_object\"");
            if (!orderingPackage || earlyAt == std::string::npos ||
                lateAt == std::string::npos)
                result = 82;
            else if (earlyAt >= lateAt)
                result = 83;
            if (result == 0) {
                const size_t earlySectionAt = orderingBytes.find(
                    "\"early_object\"", lateAt + 1);
                const size_t lateSectionAt = orderingBytes.find(
                    "\"late_object\"", earlySectionAt + 1);
                if (earlySectionAt == std::string::npos ||
                    lateSectionAt == std::string::npos ||
                    earlySectionAt >= lateSectionAt)
                    result = 84;
            }
        }
    }


    if (result == 0) {
        const std::string manifestJson =
            "{\"format\":\"olrskin\",\"version\":8,"
            "\"profile\":\"lr2-semantic-v0.8\","
            "\"semantic_authority\":\"object parts + simple_mode\","
            "\"lr2_entry\":\"lr2/main.lr2skin\","
            "\"skin_entry\":\"skin.json\","
            "\"path_map_entry\":\"compatibility/path-map.json\","
            "\"object_count\":2,\"part_count\":3,"
            "\"destination_count\":4,\"simple_slot_count\":5,"
            "\"asset_count\":6,\"virtual_root_count\":1,"
            "\"virtual_file_count\":2,\"skipped_virtual_file_count\":3,"
            "\"unresolved_image_count\":4,\"unresolved_resource_count\":5}";
        SEOLRPackageInfo manifestInfo;
        if (!SEParseOLRManifestJson(manifestJson, manifestInfo, errorMessage) ||
            manifestInfo.formatVersion != 8 || manifestInfo.objectCount != 2 ||
            manifestInfo.semanticPartCount != 3 ||
            manifestInfo.destinationCount != 4 ||
            manifestInfo.simpleSlotCount != 5 || manifestInfo.assetCount != 6 ||
            manifestInfo.virtualRootCount != 1 ||
            manifestInfo.virtualFileCount != 2)
            result = 51;

        const std::string version8Token = "\"version\":8";
        const size_t versionAt = manifestJson.find(version8Token);
        if (result == 0 && versionAt == std::string::npos)
            result = 52;
        if (versionAt != std::string::npos) {
            std::string version10 = manifestJson;
            version10.replace(versionAt, version8Token.size(), "\"version\":10");
            if (result == 0 && SEParseOLRManifestJson(version10, manifestInfo,
                errorMessage)) result = 110;

            std::string version17 = manifestJson;
            version17.replace(versionAt, version8Token.size(), "\"version\":17");
            if (result == 0 && SEParseOLRManifestJson(version17, manifestInfo,
                errorMessage)) result = 53;

            std::string version70 = manifestJson;
            version70.replace(versionAt, version8Token.size(), "\"version\":70");
            if (result == 0 && SEParseOLRManifestJson(version70, manifestInfo,
                errorMessage)) result = 54;

            std::string duplicateVersion = manifestJson;
            duplicateVersion.insert(versionAt, "\"version\":7,");
            if (result == 0 && SEParseOLRManifestJson(duplicateVersion,
                manifestInfo, errorMessage)) result = 55;

            std::string leadingZeroVersion = manifestJson;
            leadingZeroVersion.replace(versionAt, version8Token.size(),
                "\"version\":08");
            if (result == 0 && SEParseOLRManifestJson(leadingZeroVersion,
                manifestInfo, errorMessage)) result = 58;
        }

        const std::string requiredPartCount = "\"part_count\":3,";
        const size_t partCountAt = manifestJson.find(requiredPartCount);
        if (result == 0 && partCountAt == std::string::npos)
            result = 59;
        if (partCountAt != std::string::npos) {
            std::string missingPartCount = manifestJson;
            missingPartCount.erase(partCountAt, requiredPartCount.size());
            if (result == 0 && SEParseOLRManifestJson(missingPartCount,
                manifestInfo, errorMessage)) result = 60;
        }
    }

    if (result == 0) {
        const std::string partsJson =
            "{\"format\":\"olrskin-semantic\",\"version\":8,"
            "\"objects\":{\"authority\":\"lr2-destination-parts-v0.8\",\"items\":[{"
            "\"id\":\"compound\",\"parts\":[{"
            "\"id\":\"part_a\",\"sources\":[{\"source_row\":1,"
            "\"source_command\":\"#SRC_IMAGE\"}],\"destinations\":[{"
            "\"id\":\"destination_a\",\"destination_command\":\"#DST_IMAGE\","
            "\"layout\":{\"destination_row\":2,\"transform\":{\"x\":300,\"y\":200,"
            "\"width\":64,\"height\":32,\"rotation\":0,\"blend\":1}},"
            "\"animation\":{\"frames\":["
            "{\"destination_row\":2,\"time_ms\":0,\"alpha\":0,\"transform\":{"
            "\"x\":300,\"y\":200,\"width\":64,\"height\":32,\"rotation\":0,\"blend\":1}},"
            "{\"destination_row\":3,\"time_ms\":100,\"alpha\":255,\"transform\":{"
            "\"x\":310,\"y\":205,\"width\":64,\"height\":32,\"rotation\":5,\"blend\":1}}]},"
            "\"condition\":{\"mode\":\"all\",\"timer\":{\"kind\":\"semantic\","
            "\"lr2_name\":\"JUDGETIMER_1P\"},\"loop\":800,\"all\":["
            "{\"slot\":1,\"kind\":\"semantic\",\"lr2_name\":\"CLEAROPTION_SURVIVAL\","
            "\"negated\":false},{\"slot\":2,\"kind\":\"raw\",\"lr2_op\":948}]}}]},"
            "{\"id\":\"part_b\",\"sources\":[{\"source_row\":4,"
            "\"source_command\":\"#SRC_IMAGE\"}],\"destinations\":[{"
            "\"id\":\"destination_b\",\"destination_command\":\"#DST_IMAGE\","
            "\"layout\":{\"destination_row\":5,\"transform\":{\"x\":400,\"y\":300,"
            "\"width\":80,\"height\":40,\"rotation\":10,\"blend\":2}},"
            "\"animation\":{\"frames\":["
            "{\"destination_row\":5,\"time_ms\":25,\"alpha\":128,\"transform\":{"
            "\"x\":400,\"y\":300,\"width\":80,\"height\":40,\"rotation\":10,\"blend\":2}},"
            "{\"destination_row\":6,\"time_ms\":75,\"alpha\":64,\"transform\":{"
            "\"x\":410,\"y\":305,\"width\":80,\"height\":40,\"rotation\":15,\"blend\":2}}]},"
            "\"condition\":{\"mode\":\"all\",\"timer\":null,\"loop\":null,\"all\":["
            "{\"slot\":2,\"kind\":\"raw\",\"lr2_op\":39}]}}]}]}]},"
            "\"simple_mode\":{\"authority\":\"lr2-source-v0.4\",\"slots\":[]}}";
        const std::string partsInput =
            "#SRC_IMAGE,0,0,0,0,16,16,1,1,0,0,0,0,0\r\n"
            "#DST_IMAGE,0,0,1,2,3,4,0,255,255,255,255,0,0,0,0,0,0,0,0,0\r\n"
            "#DST_IMAGE,0,50,5,6,7,8,0,128,255,255,255,0,0,0,0,111,22,3,4,5\n"
            "#SRC_IMAGE,0,1,0,0,16,16,1,1,0,0,0,0,0\r\n"
            "#DST_IMAGE,0,0,10,20,30,40,0,255,255,255,255,0,0,0,0,,,,,\r\n"
            "#DST_IMAGE,0,50,15,25,35,45,0,192,255,255,255,0,0,0,0,222,33,6,7,8\n"
            "#RAW_PASSTHROUGH,keep,exact";
        const std::string partsExpected =
            "#SRC_IMAGE,0,0,0,0,16,16,1,1,0,0,0,0,0\r\n"
            "#DST_IMAGE,0,0,300,200,64,32,0,0,255,255,255,1,0,0,0,800,46,119,948,0\r\n"
            "#DST_IMAGE,0,100,310,205,64,32,0,255,255,255,255,1,0,5,0,111,22,3,4,5\n"
            "#SRC_IMAGE,0,1,0,0,16,16,1,1,0,0,0,0,0\r\n"
            "#DST_IMAGE,0,25,400,300,80,40,0,128,255,255,255,2,0,10,0,,,,39,\r\n"
            "#DST_IMAGE,0,75,410,305,80,40,0,64,255,255,255,2,0,15,0,222,33,6,7,8\n"
            "#RAW_PASSTHROUGH,keep,exact";
        std::string partsCompiled;
        int partsSimpleCount = 0;
        int partsObjectCount = 0;
        int partsFrameCount = 0;
        if (!SECompileOLRSemantics(partsJson, partsInput, partsCompiled,
            partsSimpleCount, partsObjectCount, partsFrameCount, errorMessage))
            result = 44;
        else if (partsSimpleCount != 0 || partsObjectCount != 1 ||
            partsFrameCount != 4)
            result = 56;
        else if (partsCompiled != partsExpected)
            result = 57;

        const std::string multiSourceJson =
            "{\"format\":\"olrskin-semantic\",\"version\":8,"
            "\"objects\":{\"authority\":\"lr2-destination-parts-v0.8\",\"items\":[{"
            "\"id\":\"multi_source\",\"parts\":[{\"id\":\"part_1\",\"sources\":["
            "{\"source_row\":1,\"source_command\":\"#SRC_IMAGE\"},"
            "{\"source_row\":2,\"source_command\":\"#SRC_BUTTON\"}],"
            "\"destinations\":[{\"id\":\"destination_1\","
            "\"destination_command\":\"#DST_IMAGE\","
            "\"layout\":{\"destination_row\":3,\"transform\":{\"x\":1,\"y\":2,"
            "\"width\":3,\"height\":4,\"rotation\":0,\"blend\":0}},"
            "\"animation\":{\"frames\":[{\"destination_row\":3,\"time_ms\":0,"
            "\"alpha\":255,\"transform\":{\"x\":1,\"y\":2,\"width\":3,"
            "\"height\":4,\"rotation\":0,\"blend\":0}}]},"
            "\"condition\":{\"mode\":\"all\",\"timer\":null,\"loop\":null,"
            "\"all\":[]}}]}]}]},\"simple_mode\":{\"authority\":"
            "\"lr2-source-v0.4\",\"slots\":[]}}";
        const std::string multiSourceInput =
            "#SRC_IMAGE,0,0,0,0,16,16,1,1,0,0,0,0,0\r\n"
            "#SRC_BUTTON,0,0,0,0,16,16,1,1,0,0,42,0,0\n"
            "#DST_IMAGE,0,0,1,2,3,4,0,255,255,255,255,0,0,0,0,,,,,\r\n";
        std::string multiSourceCompiled;
        int multiSourceSimpleCount = 0;
        int multiSourceObjectCount = 0;
        int multiSourceFrameCount = 0;
        if (result == 0 && (!SECompileOLRSemantics(multiSourceJson,
            multiSourceInput, multiSourceCompiled, multiSourceSimpleCount,
            multiSourceObjectCount, multiSourceFrameCount, errorMessage) ||
            multiSourceCompiled != multiSourceInput ||
            multiSourceSimpleCount != 0 || multiSourceObjectCount != 1 ||
            multiSourceFrameCount != 1))
            result = 45;

        std::string mismatchedSourceJson = multiSourceJson;
        const std::string expectedSource =
            "\"source_command\":\"#SRC_BUTTON\"";
        const size_t sourceAt = mismatchedSourceJson.find(expectedSource);
        if (result == 0 && sourceAt == std::string::npos)
            result = 46;
        if (sourceAt != std::string::npos)
            mismatchedSourceJson.replace(sourceAt, expectedSource.size(),
                "\"source_command\":\"#SRC_SLIDER\"");
        std::string rejectedSourceOutput;
        int rejectedSourceSimpleCount = 0;
        int rejectedSourceObjectCount = 0;
        int rejectedSourceFrameCount = 0;
        if (result == 0 && SECompileOLRSemantics(mismatchedSourceJson,
            multiSourceInput, rejectedSourceOutput, rejectedSourceSimpleCount,
            rejectedSourceObjectCount, rejectedSourceFrameCount, errorMessage))
            result = 47;
        if (result == 0 && (!rejectedSourceOutput.empty() ||
            rejectedSourceSimpleCount != 0 || rejectedSourceObjectCount != 0 ||
            rejectedSourceFrameCount != 0))
            result = 48;

        std::string duplicateDestinationJson = partsJson;
        const std::string partBRow = "\"destination_row\":5";
        size_t rowAt = 0;
        int replacedRows = 0;
        while ((rowAt = duplicateDestinationJson.find(partBRow, rowAt)) !=
            std::string::npos) {
            duplicateDestinationJson.replace(rowAt, partBRow.size(),
                "\"destination_row\":2");
            rowAt += strlen("\"destination_row\":2");
            ++replacedRows;
        }
        if (result == 0 && replacedRows != 2)
            result = 49;
        std::string rejectedDuplicateOutput;
        int rejectedDuplicateSimpleCount = 0;
        int rejectedDuplicateObjectCount = 0;
        int rejectedDuplicateFrameCount = 0;
        if (result == 0 && SECompileOLRSemantics(duplicateDestinationJson,
            partsInput, rejectedDuplicateOutput, rejectedDuplicateSimpleCount,
            rejectedDuplicateObjectCount, rejectedDuplicateFrameCount,
            errorMessage))
            result = 50;
        if (result == 0 && (!rejectedDuplicateOutput.empty() ||
            rejectedDuplicateSimpleCount != 0 ||
            rejectedDuplicateObjectCount != 0 ||
            rejectedDuplicateFrameCount != 0))
            result = 61;
    }

    if (result == 0) {
        const std::string semanticJson =
            "{\"objects\":{\"authority\":\"lr2-destination-v0.7\",\"items\":[{"
            "\"id\":\"judge\",\"destination_command\":\"#DST_IMAGE\","
            "\"layout\":{\"destination_row\":1,\"transform\":{\"x\":300,\"y\":200,"
            "\"width\":64,\"height\":32,\"rotation\":0,\"blend\":1}},"
            "\"animation\":{\"frames\":["
            "{\"destination_row\":1,\"time_ms\":0,\"alpha\":0,\"transform\":{"
            "\"x\":300,\"y\":200,\"width\":64,\"height\":32,\"rotation\":0,\"blend\":1}},"
            "{\"destination_row\":2,\"time_ms\":100,\"alpha\":255,\"transform\":{"
            "\"x\":310,\"y\":205,\"width\":64,\"height\":32,\"rotation\":5,\"blend\":1}}]},"
            "\"condition\":{\"mode\":\"all\",\"timer\":{\"kind\":\"semantic\","
            "\"lr2_name\":\"JUDGETIMER_1P\"},\"loop\":800,\"all\":["
            "{\"kind\":\"semantic\",\"key\":\"Gauge\",\"value\":\"HARD\","
            "\"lr2_name\":\"CLEAROPTION_SURVIVAL\",\"negated\":false},"
            "{\"kind\":\"raw\",\"lr2_op\":948,\"label\":\"Raw LR2 OP 948\"}]}}]},"
            "\"simple_mode\":{\"authority\":\"lr2-source-v0.4\",\"slots\":[]}}";
        const std::string semanticInput =
            "#DST_IMAGE,0,0,1,2,3,4,0,255,255,255,255,0,0,0,0,0,0,0,0,0\r\n"
            "#DST_IMAGE,0,50,5,6,7,8,0,128,255,255,255,0,0,0,0,0,0,0,0,0\n"
            "#RAW_PASSTHROUGH,keep,exact";
        const std::string semanticExpected =
            "#DST_IMAGE,0,0,300,200,64,32,0,0,255,255,255,1,0,0,0,800,46,119,948,0\r\n"
            "#DST_IMAGE,0,100,310,205,64,32,0,255,255,255,255,1,0,5,0,0,0,0,0,0\n"
            "#RAW_PASSTHROUGH,keep,exact";
        std::string semanticCompiled;
        int simpleCount = 0;
        int objectCount = 0;
        int frameCount = 0;
        if (!SECompileOLRSemantics(semanticJson, semanticInput, semanticCompiled,
            simpleCount, objectCount, frameCount, errorMessage) ||
            simpleCount != 0 || objectCount != 1 || frameCount != 2 ||
            semanticCompiled != semanticExpected)
            result = 30;

        const std::string mismatchedLayout = semanticJson.substr(0,
            semanticJson.find("\"x\":300")) + "\"x\":301" +
            semanticJson.substr(semanticJson.find("\"x\":300") + 7);
        std::string rejected;
        simpleCount = objectCount = frameCount = 0;
        if (result == 0 && SECompileOLRSemantics(mismatchedLayout,
            semanticInput, rejected, simpleCount, objectCount, frameCount,
            errorMessage)) result = 31;
        if (result == 0 && (!rejected.empty() || simpleCount != 0 ||
            objectCount != 0 || frameCount != 0)) result = 32;

        const std::string bargraphJson =
            "{\"objects\":{\"authority\":\"lr2-destination-v0.7\",\"items\":[{"
            "\"id\":\"mh_bargraph\",\"destination_command\":\"#DST_BARGRAPH\","
            "\"layout\":{\"destination_row\":1,\"transform\":{\"x\":572,\"y\":319,"
            "\"width\":21,\"height\":0,\"rotation\":0,\"blend\":1}},"
            "\"animation\":{\"frames\":[{\"destination_row\":1,\"time_ms\":1000,"
            "\"alpha\":255,\"transform\":{\"x\":572,\"y\":319,\"width\":21,"
            "\"height\":0,\"rotation\":0,\"blend\":1}}]},"
            "\"condition\":{\"mode\":\"all\",\"timer\":{\"kind\":\"raw\","
            "\"lr2_timer\":0},\"loop\":1400,\"all\":["
            "{\"kind\":\"raw\",\"lr2_op\":32,\"label\":\"Raw LR2 OP 32\"},"
            "{\"kind\":\"raw\",\"lr2_op\":39,\"label\":\"Raw LR2 OP 39\"}]}}]},"
            "\"simple_mode\":{\"authority\":\"lr2-source-v0.4\",\"slots\":[]}}";
        const std::string bargraphInput =
            "#DST_BARGRAPH,0,1000,572,319,21,0,2,255,255,255,255,1,0,0,0,1400,0,32,39,0,\r\n";
        std::string bargraphCompiled;
        simpleCount = objectCount = frameCount = 0;
        if (result == 0 && (!SECompileOLRSemantics(bargraphJson,
            bargraphInput, bargraphCompiled, simpleCount, objectCount,
            frameCount, errorMessage) || bargraphCompiled != bargraphInput ||
            simpleCount != 0 || objectCount != 1 || frameCount != 1))
            result = 38;

        const std::string emptyOptionsJson =
            "{\"objects\":{\"authority\":\"lr2-destination-v0.7\",\"items\":[{"
            "\"id\":\"no_conditions\",\"destination_command\":\"#DST_IMAGE\","
            "\"layout\":{\"destination_row\":1,\"transform\":{\"x\":1,\"y\":2,"
            "\"width\":3,\"height\":4,\"rotation\":0,\"blend\":0}},"
            "\"animation\":{\"frames\":[{\"destination_row\":1,\"time_ms\":0,"
            "\"alpha\":255,\"transform\":{\"x\":1,\"y\":2,\"width\":3,"
            "\"height\":4,\"rotation\":0,\"blend\":0}}]},"
            "\"condition\":{\"mode\":\"all\",\"timer\":{\"kind\":\"raw\","
            "\"lr2_timer\":0},\"loop\":0,\"all\":[]}}]},"
            "\"simple_mode\":{\"authority\":\"lr2-source-v0.4\",\"slots\":[]}}";
        const std::string emptyOptionsInput =
            "#DST_IMAGE,0,0,1,2,3,4,0,255,255,255,255,0,0,0,0,0,0,,,\r\n";
        std::string emptyOptionsCompiled;
        simpleCount = objectCount = frameCount = 0;
        if (result == 0 && (!SECompileOLRSemantics(emptyOptionsJson,
            emptyOptionsInput, emptyOptionsCompiled, simpleCount, objectCount,
            frameCount, errorMessage) ||
            emptyOptionsCompiled != emptyOptionsInput))
            result = 39;
    }
    if (result == 0) {
        std::ifstream asset(extractedPath +
            "\\vfs\\LR2files\\Theme\\Test\\note\\blue.png",
            std::ios::binary);
        const std::vector<unsigned char> extractedBytes(
            (std::istreambuf_iterator<char>(asset)),
            std::istreambuf_iterator<char>());
        if (extractedBytes.size() != sizeof(assetBytes) ||
            memcmp(extractedBytes.data(), assetBytes, sizeof(assetBytes)) != 0)
            result = 12;
    }

    if (result == 0) {
        const std::string compileJson =
            "{\"simple_mode\":{\"authority\":\"lr2-source-v0.4\",\"slots\":["
            "{\"id\":\"note\",\"category\":\"notes\","
            "\"source_command\":\"#SRC_NOTE\",\"source_row\":2,"
            "\"asset\":{\"gr\":7,\"x\":8,\"y\":9,\"width\":40,"
            "\"height\":20,\"div_x\":2,\"div_y\":1,\"cycle\":120}},"
            "{\"id\":\"gauge\",\"category\":\"gauge\","
            "\"source_command\":\"#SRC_GROOVEGAUGE\",\"source_row\":3,"
            "\"asset\":{\"gr\":8,\"x\":1,\"y\":2,\"width\":80,"
            "\"height\":10,\"div_x\":4,\"div_y\":1,\"cycle\":240}}]}}";
        const std::string compileInput =
            "#RAW_PASSTHROUGH,keep,exact\r\n"
            "#SRC_NOTE,1,0,0,0,16,16,1,1,0,77,88,99\n"
            "#SRC_GROOVEGAUGE,0,0,0,0,20,10,1,1,0,55,3,4\r\n"
            "#DST_NOTE,1,0,100,200,16,16,0,255";
        std::string compiled;
        int compiledCount = 0;
        if (!SECompileOLRSimpleMode(compileJson, compileInput, compiled,
            compiledCount, errorMessage) || compiledCount != 2 ||
            compiled != "#RAW_PASSTHROUGH,keep,exact\r\n"
                "#SRC_NOTE,1,7,8,9,40,20,2,1,120,77,88,99\n"
                "#SRC_GROOVEGAUGE,0,8,1,2,80,10,4,1,240,55,3,4\r\n"
                "#DST_NOTE,1,0,100,200,16,16,0,255")
            result = 25;
        std::string rejectedOutput;
        int rejectedCount = 0;
        const std::string mismatchedJson = compileJson.substr(0,
            compileJson.find("#SRC_NOTE")) + "#SRC_MINE" +
            compileJson.substr(compileJson.find("#SRC_NOTE") + 9);
        if (result == 0 && SECompileOLRSimpleMode(mismatchedJson,
            compileInput, rejectedOutput, rejectedCount, errorMessage))
            result = 26;
        if (result == 0 && (!rejectedOutput.empty() || rejectedCount != 0))
            result = 27;

        const std::string legacyCropJson =
            "{\"simple_mode\":{\"authority\":\"lr2-source-v0.4\",\"slots\":["
            "{\"id\":\"legacy-crop\",\"category\":\"gear\","
            "\"source_command\":\"#SRC_IMAGE\",\"source_row\":1,"
            "\"asset\":{\"gr\":2,\"x\":0,\"y\":0,\"width\":-1,"
            "\"height\":-1,\"div_x\":1,\"div_y\":1,\"cycle\":0}}]}}";
        const std::string legacyCropInput =
            "#SRC_IMAGE,0,2,0,0,-1,-1,1,1,0,0,0,0,0\r\n";
        std::string legacyCropCompiled;
        int legacyCropCount = -1;
        if (result == 0 && (!SECompileOLRSimpleMode(legacyCropJson,
            legacyCropInput, legacyCropCompiled, legacyCropCount,
            errorMessage) || legacyCropCount != 0 ||
            legacyCropCompiled != legacyCropInput))
            result = 42;
    }

    if (result == 0) {
        std::string resolved;
        if (SEResolveSkinResourcePath(
            "vfs/LR2files/Theme/Test/note/*.png",
            extractedMain.c_str(), extractedMain.c_str(), resolved) !=
                SESkinResourcePathResult::Resolved ||
            _stricmp(resolved.c_str(),
                (extractedPath +
                    "\\vfs\\LR2files\\Theme\\Test\\note\\*.png").c_str()) != 0)
            result = 22;
    }

    if (result == 0) {
        // LR2 image fonts may live only inside Graphfont.dxa while the skin
        // addresses Graphfont/font.lr2font. Win32 cannot enumerate that leaf,
        // so the imported vfs namespace must still anchor the logical path for
        // the archive-aware loader. This does not assert support for every
        // legacy DXA format version.
        const std::filesystem::path fontFolder =
            std::filesystem::path(extractedPath) / "vfs" / "LR2files" /
            "Theme" / "Test" / "Font";
        std::error_code fontError;
        std::filesystem::create_directories(fontFolder, fontError);
        if (fontError) result = 62;
        if (result == 0) {
            std::ofstream archive(fontFolder / "Graphfont.dxa",
                std::ios::binary | std::ios::trunc);
            archive << "archive-backed font fixture";
            if (!archive) result = 63;
        }
        const std::string requested =
            "vfs/LR2files/Theme/Test/Font/Graphfont/font.lr2font";
        std::string expected = (fontFolder /
            "Graphfont/font.lr2font").lexically_normal().string();
        std::replace(expected.begin(), expected.end(), '/', '\\');
        std::string resolved;
        if (result == 0 &&
            (std::filesystem::exists(expected, fontError) || fontError))
            result = 64;
        if (result == 0 && SEResolveSkinResourcePath(requested.c_str(),
            extractedMain.c_str(), extractedMain.c_str(), resolved) !=
                SESkinResourcePathResult::Resolved)
            result = 66;
        if (result == 0 &&
            _stricmp(resolved.c_str(), expected.c_str()) != 0)
            result = 67;
        std::string rejectedTraversal;
        if (result == 0 && SEResolveSkinResourcePath(
            "vfs/LR2files/../../main.lr2skin",
            extractedMain.c_str(), extractedMain.c_str(), rejectedTraversal) !=
                SESkinResourcePathResult::Rejected)
            result = 70;
        if (result == 0 && !rejectedTraversal.empty())
            result = 71;
        const char* rejectedAliases[] = {
            "vfs/LR2files./../../main.lr2skin",
            "vfs/LR2files /../../main.lr2skin",
            "vfs./LR2files/../../main.lr2skin",
            "vfs /LR2files/../../main.lr2skin",
            "vfs/Other/../../main.lr2skin",
        };
        for (const char* rejectedAlias : rejectedAliases) {
            std::string rejectedAliasPath;
            if (result == 0 && SEResolveSkinResourcePath(rejectedAlias,
                extractedMain.c_str(), extractedMain.c_str(), rejectedAliasPath) !=
                    SESkinResourcePathResult::Rejected)
                result = 73;
            if (result == 0 && !rejectedAliasPath.empty())
                result = 74;
        }
        std::filesystem::remove_all(fontFolder, fontError);
        if (result == 0 && fontError) result = 65;
    }

    if (result == 0) {
        // "vfs" is reserved only inside an imported OLR workspace. Ordinary
        // LR2 skins may already use that folder name and must retain the legacy
        // owner-relative lookup behavior.
        const std::filesystem::path normalSkinFolder =
            std::filesystem::path(root) / "normal-skin";
        const std::filesystem::path normalAssetFolder =
            normalSkinFolder / "vfs" / "Other";
        std::error_code normalError;
        std::filesystem::create_directories(normalAssetFolder, normalError);
        if (normalError) result = 75;
        const std::filesystem::path normalAsset = normalAssetFolder / "image.png";
        if (result == 0) {
            std::ofstream asset(normalAsset, std::ios::binary | std::ios::trunc);
            asset << "ordinary LR2 relative vfs fixture";
            if (!asset) result = 76;
        }
        std::string normalResolved;
        const std::string normalMain = (normalSkinFolder / "main.lr2skin").string();
        if (result == 0 && SEResolveSkinResourcePath(
            "vfs/Other/image.png", normalMain.c_str(), normalMain.c_str(),
            normalResolved) != SESkinResourcePathResult::Resolved)
            result = 77;
        if (result == 0 && std::filesystem::path(normalResolved) !=
            std::filesystem::absolute(normalAsset, normalError).lexically_normal())
            result = 78;
        if (result == 0 && normalError) result = 79;
    }

    SEOLRLr2ExportInfo exportInfo;
    if (result == 0 && (!SEIsOLRVirtualWorkspace(extractedMain.c_str()) ||
        !SEExportOLRWorkspaceToLR2(extractedMain.c_str(), materializedPath.c_str(),
            exportInfo, errorMessage)))
        result = 13;
    if (result == 0 && exportInfo.copiedFileCount != 2) result = 14;
    if (result == 0 && exportInfo.rewrittenVirtualPathCount != 4) result = 94;
    if (result == 0 && std::filesystem::path(exportInfo.mainSkinPath) !=
        std::filesystem::path(materializedPath) /
            "LR2files/Theme/Test/play.lr2skin")
        result = 21;
    if (result == 0) {
        std::ifstream compiled(exportInfo.mainSkinPath, std::ios::binary);
        const std::string compiledBytes((std::istreambuf_iterator<char>(compiled)),
            std::istreambuf_iterator<char>());
        std::string lowerCompiledBytes = compiledBytes;
        std::transform(lowerCompiledBytes.begin(), lowerCompiledBytes.end(),
            lowerCompiledBytes.begin(), [](unsigned char ch) {
                return static_cast<char>(std::tolower(ch));
            });
        if (lowerCompiledBytes.find("vfs/lr2files/") != std::string::npos ||
            lowerCompiledBytes.find("vfs\\lr2files\\") != std::string::npos ||
            compiledBytes.find("#INFORMATION,0,OLR self test,SkinEditor,LR2files\\Theme\\Test\\note\\blue.png,,1280,720") ==
                std::string::npos ||
            compiledBytes.find("LR2files\\Theme\\Test\\note\\*.png") ==
                std::string::npos ||
            compiledBytes.find("LR2files\\Theme\\Test\\note\\blue.png") ==
                std::string::npos ||
            compiledBytes.find(
                "#IMAGE,LR2files\\Theme\\Test\\assets\\simple-note.png") ==
                    std::string::npos)
            result = 15;
    }
    if (result == 0 && !std::filesystem::is_regular_file(
        std::filesystem::path(materializedPath) / "INSTALL.txt"))
        result = 95;
    if (result == 0) {
        char originalDirectory[MAX_PATH] = {};
        SkinManage exportedSkins = {};
        const bool initialized = InitSkinData(&exportedSkins) != 0;
        const bool capturedDirectory = GetCurrentDirectoryA(
            MAX_PATH, originalDirectory) != 0;
        const bool changedDirectory = capturedDirectory &&
            SetCurrentDirectoryA(materializedPath.c_str()) != FALSE;
        if (!initialized || !changedDirectory) result = 96;
        if (result == 0) {
            MakeSkinList(&exportedSkins, CSTR("LR2files\\Theme\\"));
            if (exportedSkins.Count != 1 ||
                exportedSkins.Data[0].title.isDiff("OLR self test") ||
                exportedSkins.Data[0].targetX != 1280 ||
                exportedSkins.Data[0].targetY != 720 ||
                exportedSkins.Data[0].skinFile.isDiff(
                    "LR2files\\Theme\\Test\\play.lr2skin"))
                result = 97;
        }
        if (changedDirectory && !SetCurrentDirectoryA(originalDirectory) &&
            result == 0)
            result = 98;
        if (initialized) ResetSkinData(&exportedSkins);
    }
    if (result == 0) {
        std::ifstream simpleAsset(std::filesystem::path(exportInfo.mainSkinPath)
            .parent_path() / "assets/simple-note.png", std::ios::binary);
        const std::vector<unsigned char> simpleBytes(
            (std::istreambuf_iterator<char>(simpleAsset)),
            std::istreambuf_iterator<char>());
        if (simpleBytes.size() != sizeof(assetBytes) ||
            memcmp(simpleBytes.data(), assetBytes, sizeof(assetBytes)) != 0)
            result = 23;
    }

    if (result == 0) {
        // V0.9 keeps the original file-local customization and include graph
        // when semantic compilation leaves the compatibility script unchanged.
        const std::string originalMain =
            "#INFORMATION,0,Original include main,SkinEditor,.\\VFS\\lr2files\\Theme\\Test\\note\\blue.png,,1280,720\r\n"
            "#RESOLUTION,1\r\n"
            "#CUSTOMOPTION,PLAY SIDE,900,1P,2P\r\n"
            "#IF,900\r\n"
            "#INCLUDE,parts\\play.csv\r\n"
            "#ENDIF\r\n";
        const std::filesystem::path originalMainPath =
            std::filesystem::path(virtualRoot) / "play.lr2skin";
        const std::filesystem::path originalPartPath =
            std::filesystem::path(virtualRoot) / "parts" / "play.csv";
        std::error_code preserveError;
        std::filesystem::create_directories(originalPartPath.parent_path(),
            preserveError);
        if (preserveError) result = 85;
        if (result == 0) {
            std::ofstream mainOutput(originalMainPath,
                std::ios::binary | std::ios::trunc);
            mainOutput.write(originalMain.data(), originalMain.size());
            std::ofstream partOutput(originalPartPath,
                std::ios::binary | std::ios::trunc);
            partOutput << "#HELPFILE,.\\vFs\\LR2FILES\\Theme\\Test\\help.txt\r\n"
                "#SRC_IMAGE,0,0,0,0,16,16,1,1,0,0\r\n";
            if (!mainOutput || !partOutput) result = 86;
        }

        SEOLRSkinDocument preservationDocument = document;
        preservationDocument.assets.clear();
        preservationDocument.preserveOriginalMainWhenUnchanged = true;
        preservationDocument.lr2CompatibilityBaseline =
            preservationDocument.lr2Script;
        SEOLRPackageInfo preservationInfo;
        if (result == 0 && (!SEWriteOLRSkinPackage(
            preservationPackagePath.c_str(), preservationDocument,
            preservationInfo, errorMessage) ||
            !preservationInfo.preservesOriginalMainWhenUnchanged))
            result = 87;

        std::string preservationMain;
        if (result == 0 && (!SEExtractOLRSkinPackage(
            preservationPackagePath.c_str(), preservationExtractedPath.c_str(),
            preservationMain, preservationInfo, errorMessage) ||
            !preservationInfo.preservesOriginalMainWhenUnchanged))
            result = 88;
        SEOLRLr2ExportInfo preservationExportInfo;
        if (result == 0 && (!SEExportOLRWorkspaceToLR2(
            preservationMain.c_str(), preservationMaterializedPath.c_str(),
            preservationExportInfo, errorMessage) ||
            !preservationExportInfo.preservedOriginalMain))
            result = 89;
        if (result == 0 &&
            preservationExportInfo.rewrittenVirtualPathCount != 3)
            result = 99;
        if (result == 0) {
            std::ifstream preservedMain(preservationExportInfo.mainSkinPath,
                std::ios::binary);
            const std::string preservedBytes(
                (std::istreambuf_iterator<char>(preservedMain)),
                std::istreambuf_iterator<char>());
            const std::filesystem::path preservedPart =
                std::filesystem::path(preservationExportInfo.mainSkinPath)
                    .parent_path() / "parts" / "play.csv";
            std::string expectedOriginalMain;
            std::string prepareError;
            const std::string portableOriginalMain =
                "#INFORMATION,0,Original include main,SkinEditor,LR2files\\Theme\\Test\\note\\blue.png,,1280,720\r\n"
                "#RESOLUTION,1\r\n"
                "#CUSTOMOPTION,PLAY SIDE,900,1P,2P\r\n"
                "#IF,900\r\n"
                "#INCLUDE,LR2files\\Theme\\Test\\parts\\play.csv\r\n"
                "#ENDIF\r\n";
            std::ifstream preservedPartInput(preservedPart, std::ios::binary);
            const std::string preservedPartBytes(
                (std::istreambuf_iterator<char>(preservedPartInput)),
                std::istreambuf_iterator<char>());
            if (!SEPrepareLr2ExportResolution(portableOriginalMain, 1280, 720,
                expectedOriginalMain, prepareError) || !preservedMain ||
                preservedBytes != expectedOriginalMain ||
                preservedBytes.find("\r\n#RESOLUTION,") != std::string::npos ||
                !std::filesystem::is_regular_file(preservedPart, preserveError) ||
                preserveError || !preservedPartInput ||
                preservedPartBytes.find(
                    "#HELPFILE,LR2files\\Theme\\Test\\help.txt") ==
                        std::string::npos)
                result = 90;
        }
        if (result == 0) {
            std::ofstream editedWorkspaceMain(preservationMain,
                std::ios::binary | std::ios::app);
            editedWorkspaceMain << "$OLR_SELF_TEST_EDIT\r\n";
            if (!editedWorkspaceMain) result = 91;
        }
        SEOLRLr2ExportInfo editedExportInfo;
        if (result == 0 && (!SEExportOLRWorkspaceToLR2(
            preservationMain.c_str(),
            preservationEditedMaterializedPath.c_str(), editedExportInfo,
            errorMessage) || editedExportInfo.preservedOriginalMain))
            result = 92;
        if (result == 0) {
            std::ifstream editedMain(editedExportInfo.mainSkinPath,
                std::ios::binary);
            const std::string editedBytes(
                (std::istreambuf_iterator<char>(editedMain)),
                std::istreambuf_iterator<char>());
            if (!editedMain ||
                editedBytes.find("$OLR_SELF_TEST_EDIT") == std::string::npos ||
                editedBytes == originalMain)
                result = 93;
        }

        if (result == 0) {
            // V0.8 workspaces retain flattened file-scope markers.
            // Materialization must recreate real LR2 includes and their fresh
            // IF stacks so a child orphan #ELSE cannot consume the
            // parent Scratch Left/Right customization branch. In LR2 the
            // orphan child #ELSE is ignored because each #INCLUDE starts with
            // a fresh IF stack; after flattening it must not hide P1 lane 0.
            const std::string scopeScript =
                "#INFORMATION,0,Scope test,SkinEditor,,,1280,720\r\n"
                "#CUSTOMOPTION,Scratch,901,Left,Right\r\n"
                "#LR2FONT,note\\blue.png\r\n"
                "#IMAGE,CONTINUE\r\n"
                "#ENDOFHEADER\r\n"
                "#IF,901\r\n"
                "$OLR_FILE start\r\n"
                "#ELSE\r\n"
                "#DST_NOTE,0,0,96,640,112,24,0,255\r\n"
                "$OLR_FILE end\r\n"
                "#ENDIF\r\n"
                "#IF,902\r\n"
                "$OLR_FILE start\r\n"
                "#DST_NOTE,0,0,1072,640,112,24,0,255\r\n"
                "#IF,100\r\n"
                "#IMAGE,dummy\r\n"
                "$OLR_FILE end\r\n"
                "#ENDIF\r\n";
            std::ofstream scopeWorkspaceMain(preservationMain,
                std::ios::binary | std::ios::trunc);
            scopeWorkspaceMain.write(scopeScript.data(), scopeScript.size());
            if (!scopeWorkspaceMain) result = 103;
            scopeWorkspaceMain.close();

            const std::string scopeMaterializedPath =
                root + "\\scope-materialized";
            SEOLRLr2ExportInfo scopeExportInfo;
            if (result == 0 && !SEExportOLRWorkspaceToLR2(
                preservationMain.c_str(), scopeMaterializedPath.c_str(),
                scopeExportInfo, errorMessage))
                result = 104;
            if (result == 0) {
                std::ifstream scopeMain(scopeExportInfo.mainSkinPath,
                    std::ios::binary);
                const std::string scopeBytes(
                    (std::istreambuf_iterator<char>(scopeMain)),
                    std::istreambuf_iterator<char>());
                const std::filesystem::path scopeMainDirectory =
                    std::filesystem::path(scopeExportInfo.mainSkinPath).
                        parent_path();
                std::ifstream leftInclude(scopeMainDirectory /
                    "_olr_include_0001.csv", std::ios::binary);
                const std::string leftIncludeBytes(
                    (std::istreambuf_iterator<char>(leftInclude)),
                    std::istreambuf_iterator<char>());
                std::ifstream rightInclude(scopeMainDirectory /
                    "_olr_include_0002.csv", std::ios::binary);
                const std::string rightIncludeBytes(
                    (std::istreambuf_iterator<char>(rightInclude)),
                    std::istreambuf_iterator<char>());
                bool rootedIncludeResolvesFromLr2Root = false;
                bool bareIncludeFailsFromLr2Root = false;
                char previousDirectory[MAX_PATH] = {};
                const bool capturedDirectory = GetCurrentDirectoryA(
                    MAX_PATH, previousDirectory) != 0;
                const bool changedDirectory = capturedDirectory &&
                    SetCurrentDirectoryA(scopeMaterializedPath.c_str()) != FALSE;
                if (changedDirectory) {
                    CSTR rootedIncludePath = GetRandomFileNoError(CSTR(
                        "LR2files\\Theme\\Test\\_olr_include_0001.csv"),
                        CSTR("LR2files\\Theme\\Test\\"));
                    CSTR bareIncludePath = GetRandomFileNoError(CSTR(
                        "_olr_include_0001.csv"),
                        CSTR("LR2files\\Theme\\Test\\"));
                    FILE* rootedIncludeFile = fopen(
                        rootedIncludePath.outstr(), "rb");
                    FILE* bareIncludeFile = fopen(
                        bareIncludePath.outstr(), "rb");
                    rootedIncludeResolvesFromLr2Root =
                        rootedIncludeFile != nullptr;
                    bareIncludeFailsFromLr2Root = bareIncludeFile == nullptr;
                    if (rootedIncludeFile) fclose(rootedIncludeFile);
                    if (bareIncludeFile) fclose(bareIncludeFile);
                }
                if (changedDirectory &&
                    !SetCurrentDirectoryA(previousDirectory))
                    result = 108;
                const size_t leftBranch = scopeBytes.find("#IF,901");
                const size_t rightBranch = scopeBytes.find("#IF,902");
                if (result == 0 && (!scopeMain ||
                    !rootedIncludeResolvesFromLr2Root ||
                    !bareIncludeFailsFromLr2Root ||
                    leftBranch == std::string::npos ||
                    rightBranch == std::string::npos ||
                    leftBranch >= rightBranch ||
                    scopeBytes.find(
                        "#INCLUDE,LR2files\\Theme\\Test\\_olr_include_0001.csv\r\n") ==
                            std::string::npos ||
                    scopeBytes.find(
                        "#INCLUDE,LR2files\\Theme\\Test\\_olr_include_0002.csv\r\n") ==
                            std::string::npos ||
                    scopeBytes.find("#INCLUDE,_olr_include_") !=
                        std::string::npos ||
                    scopeBytes.find(
                        "#LR2FONT,LR2files\\Theme\\Test\\note\\blue.png\r\n") ==
                            std::string::npos ||
                    scopeBytes.find("#IMAGE,CONTINUE\r\n") ==
                        std::string::npos ||
                    scopeBytes.find("$OLR_FILE") != std::string::npos ||
                    scopeBytes.find("#DST_NOTE,0") != std::string::npos ||
                    !leftInclude || !rightInclude ||
                    leftIncludeBytes.find("#ELSE\r\n") ==
                            std::string::npos ||
                    leftIncludeBytes.find(
                        "#DST_NOTE,0,0,96,640,112,24,0,255\r\n") ==
                            std::string::npos ||
                    rightIncludeBytes.find(
                        "#DST_NOTE,0,0,1072,640,112,24,0,255\r\n") ==
                            std::string::npos ||
                    rightIncludeBytes.find("#IF,100\r\n") ==
                        std::string::npos ||
                    rightIncludeBytes.find("#ENDIF") != std::string::npos))
                    result = 105;
            }
        }
    }

    if (result == 0) {
        SEOLRSkinDocument unsafeDocument = document;
        unsafeDocument.virtualRoots[0].logicalRoot = "LR2files/Theme/../escape";
        if (SEWriteOLRSkinPackage((root + "\\unsafe.olrskin").c_str(),
            unsafeDocument, packageInfo, errorMessage))
            result = 16;
    }

    if (result == 0 &&
        (!SEIsLr2DiscoverableExportMainPath(
            ".\\lr2FILES\\Theme\\Test\\play.LR2SKIN") ||
            SEIsLr2DiscoverableExportMainPath(
                "LR2files/Other/Test/play.lr2skin") ||
            SEIsLr2DiscoverableExportMainPath(
                "LR2files/Theme/Test/play.csv")))
        result = 101;
    if (result == 0) {
        SEOLRSkinDocument undiscoverableDocument = document;
        undiscoverableDocument.lr2ExportMainPath =
            "LR2files/Other/Test/play.lr2skin";
        if (SEWriteOLRSkinPackage(
            (root + "\\undiscoverable.olrskin").c_str(),
            undiscoverableDocument, packageInfo, errorMessage))
            result = 102;
    }

    if (result == 0) {
        std::ifstream input(packagePath, std::ios::binary);
        std::vector<unsigned char> bytes((std::istreambuf_iterator<char>(input)),
            std::istreambuf_iterator<char>());
        const auto match = std::search(bytes.begin(), bytes.end(),
            std::begin(assetBytes), std::end(assetBytes));
        if (match == bytes.end()) result = 17;
        else {
            *match ^= 0x20;
            std::ofstream output(tamperedPath, std::ios::binary | std::ios::trunc);
            output.write((const char*)bytes.data(), bytes.size());
            output.close();
            if (SEInspectOLRSkinPackage(tamperedPath.c_str(), packageInfo,
                errorMessage)) result = 18;
        }
    }

    if (result == 0) {
        std::string resolved;
        const std::string owner = virtualRoot + "\\play.lr2skin";
        if (SEResolveSkinResourcePath(
            ".\\LR2files\\Theme\\Test\\note\\*.png",
            owner.c_str(), owner.c_str(), resolved) !=
                SESkinResourcePathResult::Resolved ||
            _stricmp(resolved.c_str(),
                (virtualNoteFolder + "\\*.png").c_str()) != 0)
            result = 20;
    }

    if (result == 0) {
        // A CP932 lead byte followed by ASCII is invalid in common non-Japanese
        // Windows code pages. LR2 still treats the declaration as a byte path,
        // so resolving its already-validated virtual root must not throw.
        std::string legacySuffix = "groove\\";
        legacySuffix.push_back((char)0x81);
        legacySuffix += "(DEFAULT).dds";
        const std::string requested =
            ".\\LR2files\\Theme\\Test\\" + legacySuffix;
        const std::string expected = virtualRoot + "\\" + legacySuffix;
        const std::string owner = virtualRoot + "\\play.lr2skin";
        std::string resolved;
        if (SEResolveSkinResourcePath(requested.c_str(), owner.c_str(),
            owner.c_str(), resolved) != SESkinResourcePathResult::Resolved ||
            resolved != expected)
            result = 28;
        std::string resolvedAgain;
        if (result == 0 && (SEResolveSkinResourcePath(resolved.c_str(),
            owner.c_str(), owner.c_str(), resolvedAgain) !=
                SESkinResourcePathResult::Resolved ||
            resolvedAgain != expected))
            result = 29;
    }

    if (result == 0) {
        // The opened skin owns its LR2 virtual namespace. An unrelated
        // process-side LR2files tree with the same logical Theme must not
        // shadow the standalone folder selected by owner/main.
        const std::string ambientRoot = root + "\\ambient";
        const std::string ambientVirtualRoot = ambientRoot +
            "\\LR2files\\Theme\\Test";
        std::error_code createError;
        std::filesystem::create_directories(ambientVirtualRoot, createError);
        if (createError) result = 30;

        char originalDirectory[MAX_PATH] = {};
        const DWORD originalLength = GetCurrentDirectoryA(
            MAX_PATH, originalDirectory);
        if (result == 0 &&
            (originalLength == 0 || originalLength >= MAX_PATH))
            result = 31;

        bool changedDirectory = false;
        if (result == 0) {
            changedDirectory = SetCurrentDirectoryA(ambientRoot.c_str()) != FALSE;
            if (!changedDirectory) result = 32;
        }

        const std::string owner = virtualRoot + "\\play.lr2skin";
        std::string resolved;
        if (result == 0 &&
            (SEResolveSkinResourcePath(
                ".\\LR2files\\Theme\\Test\\note\\*.png",
                owner.c_str(), owner.c_str(), resolved) !=
                    SESkinResourcePathResult::Resolved ||
                _stricmp(resolved.c_str(),
                    (virtualNoteFolder + "\\*.png").c_str()) != 0))
            result = 33;

        if (changedDirectory &&
            !SetCurrentDirectoryA(originalDirectory) && result == 0)
            result = 34;
    }

    // Optional local regression hook for a real user package. CI remains
    // deterministic because the variable is normally absent; diagnostics can
    // still exercise the exact archive through the production import core.
    const char* externalPackage = std::getenv("SKINEDITOR_TEST_OLR_PACKAGE");
    if (result == 0 && externalPackage && *externalPackage) {
        const std::string externalImportPath = root + "\\external-import";
        const std::string externalExportPath = root + "\\external-export";
        std::string externalMain;
        SEOLRPackageInfo externalInfo;
        if (!SEExtractOLRSkinPackage(externalPackage,
            externalImportPath.c_str(), externalMain, externalInfo,
            errorMessage) || externalMain.empty())
            result = 43;
        bool externalHadFileScopes = false;
        if (result == 0) {
            std::ifstream externalSource(externalMain, std::ios::binary);
            const std::string externalSourceBytes(
                (std::istreambuf_iterator<char>(externalSource)),
                std::istreambuf_iterator<char>());
            if (!externalSource) result = 106;
            else externalHadFileScopes = externalSourceBytes.find(
                "$OLR_FILE start") != std::string::npos;
        }
        SEOLRLr2ExportInfo externalExportInfo;
        if (result == 0 && externalInfo.formatVersion >= 2 &&
            !SEExportOLRWorkspaceToLR2(externalMain.c_str(),
            externalExportPath.c_str(), externalExportInfo, errorMessage))
            result = 68;
        if (result == 0 && externalInfo.formatVersion >= 8 &&
            externalExportInfo.copiedFileCount != externalInfo.assetCount)
            result = 72;
        if (result == 0 && externalInfo.formatVersion >= 2) {
            std::ifstream exportedMain(externalExportInfo.mainSkinPath,
                std::ios::binary);
            const std::string exportedBytes(
                (std::istreambuf_iterator<char>(exportedMain)),
                std::istreambuf_iterator<char>());
            std::string lowerExportedBytes = exportedBytes;
            std::transform(lowerExportedBytes.begin(), lowerExportedBytes.end(),
                lowerExportedBytes.begin(), [](unsigned char ch) {
                    return static_cast<char>(std::tolower(ch));
                });
            if (!exportedMain || exportedBytes.empty() ||
                lowerExportedBytes.find("vfs/lr2files/") != std::string::npos ||
                lowerExportedBytes.find("vfs\\lr2files\\") != std::string::npos)
                result = 69;
            if (result == 0 && externalHadFileScopes &&
                !externalExportInfo.preservedOriginalMain) {
                const std::filesystem::path generatedInclude =
                    std::filesystem::path(externalExportInfo.mainSkinPath).
                        parent_path() / "_olr_include_0001.csv";
                std::error_code includeError;
                const size_t includeNameAt = lowerExportedBytes.find(
                    "_olr_include_0001.csv");
                const size_t includeLineStart = includeNameAt ==
                    std::string::npos ? std::string::npos :
                    lowerExportedBytes.rfind('\n', includeNameAt);
                const size_t includeLineBegin = includeLineStart ==
                    std::string::npos ? 0 : includeLineStart + 1;
                const size_t includeLineEnd = includeNameAt ==
                    std::string::npos ? std::string::npos :
                    lowerExportedBytes.find_first_of("\r\n", includeNameAt);
                const std::string includeLine = includeNameAt ==
                    std::string::npos ? std::string() :
                    lowerExportedBytes.substr(includeLineBegin,
                        includeLineEnd == std::string::npos ?
                            std::string::npos :
                            includeLineEnd - includeLineBegin);
                if (exportedBytes.find("$OLR_FILE") != std::string::npos ||
                    includeLine.rfind("#include,lr2files\\", 0) != 0 ||
                    lowerExportedBytes.find(
                        "#include,_olr_include_") != std::string::npos ||
                    !std::filesystem::is_regular_file(generatedInclude,
                        includeError) || includeError)
                    result = 107;
            }
        }
    }

    std::error_code cleanupError;
    std::filesystem::remove_all(root, cleanupError);
    if (result == 0 && cleanupError) result = 19;
    return result;
}

int RunUiCatalogSelfTest() {
    if (kSEUIWindowSpecCount != static_cast<std::size_t>(SEUIWindowId::Count))
        return 1;
    if (kSEUISurfaceSpecCount != static_cast<std::size_t>(SEUISurfaceId::Count))
        return 2;

    std::set<std::string> keys;
    std::set<std::string> titles;
    for (std::size_t index = 0; index < kSEUIWindowSpecCount; ++index) {
        const SEUIWindowSpec& spec = kSEUIWindowSpecs[index];
        if (static_cast<std::size_t>(spec.id) != index) return 3;
        if (!spec.key || !*spec.key || !spec.title || !*spec.title ||
            !spec.purpose || !*spec.purpose || !spec.ownerFunction ||
            !*spec.ownerFunction || !spec.group || !*spec.group ||
            !spec.defaultDock || !*spec.defaultDock)
            return 4;
        if (std::strstr(spec.title, "##")) return 5;
        if (!keys.insert(spec.key).second) return 6;
        if (!titles.insert(spec.title).second) return 7;
    }
    for (std::size_t index = 0; index < kSEUISurfaceSpecCount; ++index) {
        const SEUISurfaceSpec& spec = kSEUISurfaceSpecs[index];
        if (static_cast<std::size_t>(spec.id) != index) return 8;
        if (!spec.key || !*spec.key || !spec.title || !*spec.title ||
            !spec.purpose || !*spec.purpose || !spec.ownerFunction ||
            !*spec.ownerFunction || !spec.kind || !*spec.kind)
            return 9;
        if (!keys.insert(spec.key).second) return 10;
        if (!titles.insert(spec.title).second) return 11;
    }

    char title[128];
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::ImageManager, 7);
    if (std::strcmp(title, "Image Manager##image-manager-7") != 0) return 12;
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::History, 3);
    if (std::strcmp(title, "History##history-3") != 0) return 13;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::ObjectBrowser).defaultDock,
        "left-browser") != 0) return 14;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::ObjectInspector).defaultDock,
        "left-inspector") != 0) return 15;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::TextEditor).defaultDock,
        "center-tabs") != 0) return 16;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::ObjectProperty).group,
        "Advanced") != 0) return 17;
    FormatSEUIWindowTitle(title, sizeof(title), SEUIWindowId::TimerControl, 5);
    if (std::strcmp(title, "Timer Control##timer-control-5") != 0) return 18;
    if (std::strcmp(SEUIWindowSpecFor(SEUIWindowId::TimerControl).defaultDock,
        "right-lower") != 0) return 19;
    const SEUISurfaceSpec& saveOlrSkin =
        SEUISurfaceSpecFor(SEUISurfaceId::SaveOlrSkin);
    if (std::strcmp(saveOlrSkin.key, "save-olrskin") != 0 ||
        std::strcmp(saveOlrSkin.title, "Save OLRskin") != 0 ||
        std::strcmp(saveOlrSkin.ownerFunction,
            "WORKSPACE::drawSaveOlrSkin") != 0)
        return 20;

    return 0;
}

int RunSkinBrowserSelfTest() {
    if (!SEIsSkinScriptFilename("skin.lr2skin")) return 1;
    if (!SEIsSkinScriptFilename("skin.LR2SS")) return 2;
    if (SEIsSkinScriptFilename("skin.txt")) return 3;

    char temporaryRoot[MAX_PATH] = {};
    char scanRoot[MAX_PATH] = {};
    if (!GetTempPathA(MAX_PATH, temporaryRoot)) return 4;
    if (!GetTempFileNameA(temporaryRoot, "SEB", 0, scanRoot)) return 5;
    DeleteFileA(scanRoot);
    if (!CreateDirectoryA(scanRoot, NULL)) return 6;

    const std::string nested = std::string(scanRoot) + "\\nested";
    const std::string rootSkin = std::string(scanRoot) + "\\root.LR2SKIN";
    const std::string nestedSkin = nested + "\\nested.lr2ss";
    const std::string ignored = std::string(scanRoot) + "\\readme.txt";
    int result = 0;
    if (!CreateDirectoryA(nested.c_str(), NULL)) result = 7;

    const auto writeEmptyFile = [](const std::string& path) {
        FILE* file = fopen(path.c_str(), "wb");
        if (!file) return false;
        fclose(file);
        return true;
    };
    if (result == 0 && (!writeEmptyFile(rootSkin) ||
        !writeEmptyFile(nestedSkin) || !writeEmptyFile(ignored))) result = 8;

    if (result == 0) {
        const SESkinFolderScanResult scan = SEScanSkinFolder(scanRoot);
        if (!scan.success) result = 9;
        else if (scan.files.size() != 2) result = 10;
        else if (_stricmp(scan.files[0].c_str(), nestedSkin.c_str()) != 0 ||
            _stricmp(scan.files[1].c_str(), rootSkin.c_str()) != 0) result = 11;
    }
    if (result == 0) {
        const std::string missing = std::string(scanRoot) + "\\missing";
        if (SEScanSkinFolder(missing.c_str()).success) result = 12;
    }

    if (result == 0) {
        FILE* skinFile = fopen(rootSkin.c_str(), "wb");
        if (!skinFile) result = 13;
        else {
            fputs("#INFORMATION,0,Resolution test,Self test,,,,\r\n", skinFile);
            fputs("#RESOLUTION,1280,720\r\n", skinFile);
            fclose(skinFile);

            SkinManage parsedSkin = {};
            if (!InitSkinData(&parsedSkin)) result = 14;
            else {
                char originalWorkingDirectory[MAX_PATH] = {};
                if (!GetCurrentDirectoryA(MAX_PATH, originalWorkingDirectory) ||
                    !SetCurrentDirectoryA(scanRoot)) {
                    result = 15;
                }
                if (result == 0)
                    ParseLR2SkinCustom(&parsedSkin, CSTR(rootSkin.c_str()));
                if (*originalWorkingDirectory)
                    SetCurrentDirectoryA(originalWorkingDirectory);
                SESkinResolutionDecision browserResolution;
                if (result == 0 && (parsedSkin.Count != 1 ||
                    parsedSkin.Data[0].targetX != 1280 ||
                    parsedSkin.Data[0].targetY != 720 ||
                    !SEResolveSkinResolutionFile(rootSkin.c_str(),
                        browserResolution) ||
                    browserResolution.width != 1280 ||
                    browserResolution.height != 720)) result = 16;
                if (result == 0) {
                    for (int skinIndex = 1; skinIndex < 101; ++skinIndex)
                        ParseLR2SkinCustom(&parsedSkin, CSTR(rootSkin.c_str()));
                    if (parsedSkin.Count != 101 || parsedSkin.Max != 200 ||
                        !parsedSkin.Data[100].title.isSame("Resolution test") ||
                        !parsedSkin.Data[100].customs[0].op_label ||
                        parsedSkin.Data[100].customs[0].labelCapacity != 100)
                        result = 17;
                }
            }
        }
    }

    DeleteFileA(ignored.c_str());
    DeleteFileA(nestedSkin.c_str());
    DeleteFileA(rootSkin.c_str());
    RemoveDirectoryA(nested.c_str());
    RemoveDirectoryA(scanRoot);

    if (result == 0) {
        SkinManage skinData = {};
        if (!InitSkinData(&skinData)) result = 18;
        else {
            skinData.Data[0].skinFile.assign("first.lr2skin");
            skinData.Data[0].customs[0].title.assign("Option");
            skinData.Data[0].customs[0].op_label[0].assign("Choice");
            skinData.Data[0].customs[0].dst_op_count = 1;
            skinData.Data[0].custom_count = 1;
            skinData.Count = 1;
            CSTR* const labelTable = skinData.Data[0].customs[0].op_label;
            if (!ResetSkinData(&skinData)) result = 19;
            else if (skinData.Count != 0 || skinData.Data[0].skinFile.body ||
                skinData.Data[0].customs[0].title.body ||
                skinData.Data[0].customs[0].op_label != labelTable ||
                skinData.Data[0].customs[0].op_label[0].body ||
                skinData.Data[0].customs[0].dst_op_count != 0) result = 20;
        }
    }
    return result;
}

int RunPreviewSimulatorSelfTest() {
    // A native 7-key chart never rotates key lanes merely because its skin
    // draws scratch lane 0 on the right. This keeps note lane N paired with
    // key-beam timer 100+N in the editor just as it is in actual LR2 play.
    if (LR2SEGetSamplePreviewScratchSide(SKINTYPE_7KEYS, 1, 0) != 0 ||
        LR2SEGetSamplePreviewScratchSide(SKINTYPE_7KEYS, 0, 1) != 0 ||
        LR2SEGetSamplePreviewScratchSide(SKINTYPE_7KEYSBATTLE, 1, 1) != 0)
        return 25;
    if (LR2SEGetSamplePreviewScratchSide(SKINTYPE_5KEYSBATTLE, 1, 0) != 1 ||
        LR2SEGetSamplePreviewScratchSide(SKINTYPE_5KEYSBATTLE, 0, 1) != 2 ||
        LR2SEGetSamplePreviewScratchSide(SKINTYPE_5KEYSBATTLE, 1, 1) != 3)
        return 26;

    LR2SEPreviewChartNote notes[256] = {};
    int count = LR2SEBuildPreviewChart(SKINTYPE_7KEYS,
        LR2SE_PREVIEW_CHART_FULL, notes, 256);
    if (count != 180) return 1;
    if (notes[0].lane != 0 || notes[0].timingMs != 2200 ||
        notes[0].kind != LR2SE_PREVIEW_NOTE_NORMAL)
        return 2;

    bool laneSeen[20] = {};
    bool laneHasPrevious[20] = {};
    unsigned int lanePrevious[20] = {};
    int longNotes = 0;
    int mines = 0;
    unsigned int previousTiming = 0;
    for (int index = 0; index < count; ++index) {
        const LR2SEPreviewChartNote& note = notes[index];
        if (note.lane < 0 || note.lane > 7) return 3;
        if (note.timingMs < previousTiming) return 4;
        if (laneHasPrevious[note.lane] && note.timingMs <= lanePrevious[note.lane])
            return 5;
        previousTiming = note.timingMs;
        lanePrevious[note.lane] = note.timingMs;
        laneHasPrevious[note.lane] = true;
        laneSeen[note.lane] = true;
        if (note.kind == LR2SE_PREVIEW_NOTE_LONG) {
            if (note.endTimingMs <= note.timingMs) return 6;
            longNotes++;
        }
        else {
            if (note.endTimingMs != 0) return 7;
            if (note.kind == LR2SE_PREVIEW_NOTE_MINE) mines++;
        }
    }
    for (int lane = 0; lane <= 7; ++lane) {
        if (!laneSeen[lane]) return 8;
    }
    if (longNotes == 0 || mines == 0) return 9;

    count = LR2SEBuildPreviewChart(SKINTYPE_14KEYS,
        LR2SE_PREVIEW_CHART_FULL, notes, 256);
    if (count != 180) return 10;
    bool firstPlayer = false;
    bool secondPlayer = false;
    for (int index = 0; index < count; ++index) {
        firstPlayer = firstPlayer || (notes[index].lane >= 0 && notes[index].lane <= 7);
        secondPlayer = secondPlayer || (notes[index].lane >= 10 && notes[index].lane <= 17);
    }
    if (!firstPlayer || !secondPlayer) return 11;

    count = LR2SEBuildPreviewChart(SKINTYPE_9KEYS,
        LR2SE_PREVIEW_CHART_FULL, notes, 256);
    if (count != 180) return 12;
    for (int index = 0; index < count; ++index) {
        if (notes[index].lane < 1 || notes[index].lane > 9) return 13;
    }

    if (LR2SEBuildPreviewChart(SKINTYPE_14KEYS,
        LR2SE_PREVIEW_CHART_FULL, notes, 5) != 5)
        return 14;
    if (LR2SEBuildPreviewChart(SKINTYPE_7KEYS,
        LR2SE_PREVIEW_CHART_FULL, nullptr, 256) != 0)
        return 15;

    count = LR2SEBuildPreviewChart(SKINTYPE_7KEYS,
        LR2SE_PREVIEW_CHART_SIMPLE, notes, 256);
    if (count != 16) return 16;
    longNotes = 0;
    mines = 0;
    previousTiming = 0;
    for (int index = 0; index < count; ++index) {
        const LR2SEPreviewChartNote& note = notes[index];
        if (note.timingMs < previousTiming) return 17;
        previousTiming = note.timingMs;
        if (note.kind == LR2SE_PREVIEW_NOTE_LONG) longNotes++;
        if (note.kind == LR2SE_PREVIEW_NOTE_MINE) mines++;
    }
    if (longNotes == 0 || mines == 0) return 18;

    LR2SEPreviewTimelineEvent timeline[32] = {};
    count = LR2SEBuildPreviewTimeline(timeline, 32);
    if (count != 23) return 19;
    for (int index = 0; index < count - 1; ++index) {
        if (timeline[index].timingMs != (unsigned int)index * 1600U ||
            timeline[index].op != 2 || timeline[index].value != 0.0 ||
            timeline[index].terminal)
            return 20;
    }
    if (timeline[count - 1].timingMs != 34000U ||
        timeline[count - 1].op != 2 || !timeline[count - 1].terminal)
        return 21;
    if (LR2SEBuildPreviewTimeline(timeline, 2) != 2 ||
        LR2SEBuildPreviewTimeline(nullptr, 32) != 0)
        return 22;

    Timer rhythmTimer = {};
    rhythmTimer.Rhythm = -1.0;
    if (SetTimeLapse(140, &rhythmTimer) != 1 ||
        GetTimeLapse(140, &rhythmTimer) != 0.0)
        return 23;
    if (ResetTimeLapse(140, &rhythmTimer) != 1 ||
        GetTimeLapse(140, &rhythmTimer) != -1.0)
        return 24;

    if (!LR2SEShouldDrawStaticNormalSample(0, true) ||
        LR2SEShouldDrawStaticNormalSample(1, true) ||
        !LR2SEShouldDrawStaticNormalSample(2, true))
        return 25;
    if (!LR2SEShouldDrawStaticNormalSample(1, false) ||
        LR2SEShouldDrawStaticNormalSample(-1, false) ||
        LR2SEShouldDrawStaticNormalSample(3, false))
        return 26;

    if (LR2SEGetSamplePreviewScratchSide(SKINTYPE_14KEYS, 0, 1) != 0 ||
        LR2SEGetSamplePreviewScratchSide(SKINTYPE_14KEYS, 1, 0) != 0 ||
        LR2SEGetSamplePreviewScratchSide(SKINTYPE_10KEYS, 0, 1) != 0)
        return 27;
    if (LR2SEGetSamplePreviewScratchSide(SKINTYPE_5KEYSBATTLE, 1, 0) != 1 ||
        LR2SEGetSamplePreviewScratchSide(SKINTYPE_5KEYSBATTLE, 0, 1) != 2)
        return 28;

    std::unique_ptr<gameplay> preview(new gameplay());
    preview->isCourse = 1;
    preview->courseType = 0;
    preview->courseStageCount = 5;
    preview->courseStageNow = 3;
    preview->courseFilepath[0].assign("stale-course-stage.bme");
    preview->courseConnection[0] = 4;
    LR2SEResetPreviewCourseState(preview.get());
    if (preview->isCourse != 0 || preview->courseType != -1 ||
        preview->courseStageCount != 1 || preview->courseStageNow != 0)
        return 29;
    for (int stage = 0; stage < 5; ++stage) {
        if (preview->courseFilepath[stage].length() != 0) return 30;
    }
    for (int connection = 0; connection < 10; ++connection) {
        if (preview->courseConnection[connection] != 0) return 31;
    }

    return 0;
}
