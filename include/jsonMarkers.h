#pragma once

#ifndef TWIN_JSONMARKERS_H
#define TWIN_JSONMARKERS_H

#include "twinDefines.h"
#include <cppgl.h>
#include "movementEvent.h"


TWIN_NAMESPACE_BEGIN
    class JMarker {

    public:
        JMarker(const std::string &path = "");

        ~JMarker();

        std::vector<std::vector<cppgl::vec3>> getPointData() const;

        std::vector<cppgl::vec3> getFramePoints(int frame = -1) const;

        std::map<std::string, cppgl::vec3> getFrameMarkers(int frame = -1) const;

        std::vector<cppgl::vec3> getPointFrames(const std::string &name) const;

        std::vector<cppgl::vec3> getPointFrames(int id) const;

        std::vector<Event> getEvents() const;

        int getNumFrames() const;

        int getNumPoints() const;

        int getFramerate() const;

        int getSyncFrame() const;

        void setFrame(int frame);

        void drawGUI();

        void Render() const;

        static void drawClassGUI();

        friend std::ostream &operator<<(std::ostream &os, const JMarker &c3d);

        std::vector<std::string> m_markerNames;
        std::vector<std::string> m_gaitBonesNames;
        std::map<std::string, int> m_nameIdMap;

        std::string m_path;

    private:
        bool readFile(const std::string &path);

        std::vector<std::string> m_ununsedNames;

        std::vector<std::vector<cppgl::vec3>> m_pointData;
        std::vector<std::map<std::string, cppgl::vec3>> m_markerData;
        std::vector<std::map<std::string, cppgl::vec3>> m_gaitBoneData;
        std::vector<Event> m_events;
        std::vector<std::string> m_eventnames;

        int m_framesel;
        int m_numFrames;
        int m_numPoints;
        int m_framerate;

        int m_curFrame;

        bool m_err;
        bool m_draw;

        int m_id;

        int m_zeroframe;

        static int c_count;
        static bool c_gui;

        cppgl::Drawelement m_repr;
    };

TWIN_NAMESPACE_END

#endif //TWIN_JSONMARKERS_H
