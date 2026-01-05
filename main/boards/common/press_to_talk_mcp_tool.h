#ifndef PRESS_TO_TALK_MCP_TOOL_H
#define PRESS_TO_TALK_MCP_TOOL_H

#include "mcp_server.h"
#include "settings.h"

// Reusable button-to-talk mode mcp tool class

class PressToTalkMcpTool {
private:
    bool press_to_talk_enabled_;

public:
    PressToTalkMcpTool();
    
    // Initialize tool and register to mcp server

    void Initialize();
    
    // Get the current button-to-talk mode status

    bool IsPressToTalkEnabled() const;

private:
    // Mcp tool callback function

    ReturnValue HandleSetPressToTalk(const PropertyList& properties);
    
    // Internal method: Set press to talk status and save to settings

    void SetPressToTalkEnabled(bool enabled);
};

#endif // Press to talk mcp tool h 