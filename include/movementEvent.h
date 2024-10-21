#pragma once
#ifndef TWIN_MOVEMENTEVENT_H
#define TWIN_MOVEMENTEVENT_H

#include "twinDefines.h"

TWIN_NAMESPACE_BEGIN

    enum EventType : int8_t {
        LHS = 0,//left heel strike
        LTO = 2,//left toe lift off
        RHS = 1,//right heel strike
        RTO = 3,//right toe lift off
        LTS = 4,//left toe strike
        RTS = 5,//right toe strike
        ERROR = -1
    };

    class Event {
    public:
        Event() : event("NO EVENT"), frame(-1) {};

        Event(int f, const std::string &et) : event(et), frame(f) {};

        std::string event;
        int frame;

        bool operator<(const Event &rhs) const {
            return frame < rhs.frame;
        }

        friend std::ostream &operator<<(std::ostream &os, const Event &dt) {
            /*
            const std::map<EventType, std::string> names = {{EventType::ERROR, "Error"},
                                                            {EventType::LTO,    "left toe lift off"},
                                                            {EventType::RTO,    "right toe lift off"},
                                                            {EventType::RHS,    "right heel strike"},
                                                            {EventType::LHS,    "right heel strike"},
                                                            {EventType::LTS,    "left toe strike"},
                                                            {EventType::RTS,    "right toe strike"}};
            */
            os << "[Event] at frame: " << dt.frame << " type: " << dt.event;//names.at(dt.event);
            return os;
        }
    };

TWIN_NAMESPACE_END

#endif //TWIN_MOVEMENTEVENT_H
