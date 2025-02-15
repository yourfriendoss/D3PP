//
// Created by unknown on 8/9/21.
//

#ifndef D3PP_EVENTENTITYPOSITIONSET_H
#define D3PP_EVENTENTITYPOSITIONSET_H
#include "EventSystem.h"
class EventEntityPositionSet : public Event {
public:
    EventEntityPositionSet();
    static constexpr DescriptorType descriptor = "Entity_Position_Set";
    [[nodiscard]] DescriptorType type() const override;
    
    int entityId;
    int mapId;
    float x;
    float y;
    float z;
    float rotation;
    float look;
    unsigned char priority;
    bool sendOwnClient;

    int Push(lua_State *L);
};
#endif //D3PP_EVENTENTITYPOSITIONSET_H
