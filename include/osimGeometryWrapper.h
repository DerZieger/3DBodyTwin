
#pragma once
//#include "simbody/SimTKcommon.h"
#include "SimTKcommon.h"
#include "twinDefines.h"

TWIN_NAMESPACE_BEGIN
//This is required to generate actual  mesh data from the decorative mesh provided
    class osimGeometryWrapper : public SimTK::DecorativeGeometryImplementation {
    public:
        osimGeometryWrapper(const SimTK::State &state);

        ~osimGeometryWrapper() {}

        void implementPointGeometry(const SimTK::DecorativePoint &geom) override;

        void implementLineGeometry(const SimTK::DecorativeLine &geom) override;

        void implementBrickGeometry(const SimTK::DecorativeBrick &geom) override;

        void implementCylinderGeometry(const SimTK::DecorativeCylinder &geom) override;

        void implementCircleGeometry(const SimTK::DecorativeCircle &geom) override;

        void implementSphereGeometry(const SimTK::DecorativeSphere &geom) override;

        void implementEllipsoidGeometry(const SimTK::DecorativeEllipsoid &geom) override;

        void implementFrameGeometry(const SimTK::DecorativeFrame &geom) override;

        void implementTextGeometry(const SimTK::DecorativeText &geom) override;

        void implementMeshGeometry(const SimTK::DecorativeMesh &geom) override;

        void implementMeshFileGeometry(const SimTK::DecorativeMeshFile &geom) override;

        void implementArrowGeometry(const SimTK::DecorativeArrow &geom) override {};  // Not handled yet by this Visualizer
        void implementTorusGeometry(const SimTK::DecorativeTorus &geom) override {};  // Not handled yet by this Visualizer
        void implementConeGeometry(const SimTK::DecorativeCone &geom) override {};    // Not handled yet by this Visualizer
        static SimTK::Vec4 getColor(const SimTK::DecorativeGeometry &geom, const SimTK::Vec3 &defaultColor = SimTK::Vec3(-1));

        struct Value {
            SimTK::PolygonalMesh mesh;
            SimTK::Vec3 color;
            SimTK::Transform trans;
            SimTK::Vec3 scale;
        };

        Value &&getLastMesh() { return std::move(lastMesh); }

    private:
        int getRepresentation(const SimTK::DecorativeGeometry &geom) const;

        unsigned short getResolution(const SimTK::DecorativeGeometry &geom) const;

        SimTK::Vec3 getScaleFactors(const SimTK::DecorativeGeometry &geom) const;

        SimTK::Transform calcX_GD(const SimTK::DecorativeGeometry &geom) const;

        const SimTK::State &state;
        Value lastMesh;
    };

TWIN_NAMESPACE_END