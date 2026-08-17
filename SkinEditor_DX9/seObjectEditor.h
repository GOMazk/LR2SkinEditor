#pragma once
#include <vector>
#include <string>

struct SEObjectGroupDef {
    std::string name;
    std::vector<std::string> commands;
};

struct SEObjectInstance {
    int group = -1;
    int ifgroup = 0;          // branch this object belongs to; 0 = unconditional
    int branchHeaderRow = -1; // #IF/#ELSEIF/#ELSE row, UI metadata only
    std::vector<int> rows;    // NEVER contains control-flow rows
    std::string name;         // editor display name; TEXT defaults from $st
    std::string editorId;     // persistent $SE_OBJECT_ID
};

struct SEUserObjectGroup {
    std::string name;
    std::vector<std::string> memberIds;
};

class SEObjectEditorModel {
public:
    bool LoadGroups(const char* path);
    void Rebuild(class WORKSPACE& ws);
    const std::vector<SEObjectGroupDef>& Groups() const { return groups; }
    const std::vector<SEObjectInstance>& Objects() const { return objects; }
    std::vector<SEObjectInstance>& ObjectsMutable() { return objects; }
    const SEObjectGroupDef* Group(int index) const;
    std::vector<int> ObjectsForGroup(int group) const;
    const std::vector<SEUserObjectGroup>& UserGroups() const { return userGroups; }
    std::vector<int> ObjectsForUserGroup(int group) const;
private:
    std::vector<SEObjectGroupDef> groups;
    std::vector<SEObjectInstance> objects;
    std::vector<SEUserObjectGroup> userGroups;
    std::string loadedPath;
};

extern SEObjectEditorModel g_seObjectEditorModel;
