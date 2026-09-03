#pragma once
#include <vector>
#include <string>

struct WORKSPACE;

struct SEObjectGroupDef {
    std::string name;
    std::vector<std::string> commands;
};
struct SEObjectInstance {
    int group = -1;
    int ifgroup = 0;          // branch this object belongs to; 0 = unconditional
    int branchHeaderRow = -1; // #IF/#ELSEIF/#ELSE row, UI metadata only
    int drawOrder = -1;       // global back-to-front rank; -1 when there is no DST
    int firstDstRow = -1;     // first DST declaration used to derive LR2 draw order
    std::vector<int> rows;    // NEVER contains control-flow rows
    std::string name;         // explicit name or command-specific symbolic source name
    std::string editorId;     // persistent $SE_OBJECT_ID
};

int SEFindObjectForRow(const std::vector<SEObjectInstance>& objects, int row);

struct SEUserObjectGroup {
    std::string name;
    std::vector<std::string> memberIds;
};

class SEObjectEditorModel {
public:
    bool LoadGroups(const char* path);
    void Rebuild(WORKSPACE& ws);
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
