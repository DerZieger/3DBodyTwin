#include "osimGeometryWrapper.h"

#include "SimTKcommon.h"
#include "glog/logging.h"

using namespace SimTK;

static const Vec3 DefaultBodyColor = Gray;
static const Vec3 DefaultPointColor = Magenta;

twin::osimGeometryWrapper::osimGeometryWrapper(const State &state) : state(state) {}

// The DecorativeGeometry's frame D is given in the body frame B, via transform
// X_BD. We want to know X_GD, the pose of the geometry in Ground, which we get
// via X_GD=X_GB*X_BD.
Transform twin::osimGeometryWrapper::calcX_GD(const DecorativeGeometry &geom) const {
    // const MobilizedBody& mobod =
    //    matter.getMobilizedBody(MobilizedBodyIndex(geom.getBodyId()));
    // const Transform& X_GB  = mobod.getBodyTransform(state);
    const Transform &X_BD = geom.getTransform();
    // return X_GB*X_BD;
    return X_BD;
}


// We're going to draw three short lines aligned with the body axes
// that intersect at the point.
void twin::osimGeometryWrapper::implementPointGeometry(const SimTK::DecorativePoint &geom) {
    // const MobilizedBody& mobod =
    //     matter.getMobilizedBody(MobilizedBodyIndex(geom.getBodyId()));
    // const Transform& X_GB  = mobod.getBodyTransform(state);
    // const Transform& X_BD  = geom.getTransform();
    // const Transform X_GD = X_GB*X_BD;
    // const Vec3 p_GP = X_GD*geom.getPoint();
    // const Real thickness =
    //     geom.getLineThickness() == -1 ? Real(1) : geom.getLineThickness();

    // const Real DefaultLength = Real(0.05); // 1/20 of a unit length
    // const Vec3 lengths = DefaultLength * getScaleFactors(geom);
    // const Vec4 color = getColor(geom, DefaultPointColor);

    // protocol.drawLine(p_GP - lengths[0]*X_GB.x(),
    //         p_GP + lengths[0]*X_GB.x(), color, thickness);
    // protocol.drawLine(p_GP - lengths[1]*X_GB.y(),
    //         p_GP + lengths[1]*X_GB.y(), color, thickness);
    // protocol.drawLine(p_GP - lengths[2]*X_GB.z(),
    //         p_GP + lengths[2]*X_GB.z(), color, thickness);
    VLOG(4) << "pointgeometry" << std::endl;
}

void twin::osimGeometryWrapper::implementLineGeometry(const SimTK::DecorativeLine &geom) {
    // const Transform X_GD = calcX_GD(geom);
    // protocol.drawLine(X_GD*geom.getPoint1(), X_GD*geom.getPoint2(), getColor(geom), geom.getLineThickness() == -1 ? 1
    // : geom.getLineThickness());
    VLOG(4) << "pointgeometry" << std::endl;
}

void twin::osimGeometryWrapper::implementBrickGeometry(const SimTK::DecorativeBrick &geom) {
    //    const Transform X_GD = calcX_GD(geom);
    //    const Vec3 hlen = getScaleFactors(geom).elementwiseMultiply(geom.getHalfLengths());
    //    protocol.drawBox(X_GD, hlen, getColor(geom), getRepresentation(geom));
    VLOG(4) << "brickgeometry" << std::endl;
}

void twin::osimGeometryWrapper::implementCylinderGeometry(const SimTK::DecorativeCylinder &geom) {
    // const Transform X_GD = calcX_GD(geom);
    // const Vec3 scale = getScaleFactors(geom);
    // protocol.drawCylinder(X_GD, Vec3(scale[0]*geom.getRadius(),
    //            scale[1]*geom.getHalfHeight(),
    //            scale[2]*geom.getRadius()),
    //        getColor(geom), getRepresentation(geom), getResolution(geom));
    VLOG(4) << "cylingeometry" << std::endl;
}

void twin::osimGeometryWrapper::implementCircleGeometry(const SimTK::DecorativeCircle &geom) {
    // const Transform X_GD = calcX_GD(geom);
    // const Vec3 scale = getScaleFactors(geom); // z ignored
    // protocol.drawCircle(X_GD, Vec3(scale[0]*geom.getRadius(),
    //            scale[1]*geom.getRadius(), 1),
    //        getColor(geom), getRepresentation(geom), getResolution(geom));
    VLOG(4) << "circlgeometry" << std::endl;
}

void twin::osimGeometryWrapper::implementSphereGeometry(const SimTK::DecorativeSphere &geom) {
    // const Transform X_GD = calcX_GD(geom);
    // protocol.drawEllipsoid(X_GD, geom.getRadius()*getScaleFactors(geom),
    //        getColor(geom), getRepresentation(geom), getResolution(geom));
    VLOG(4) << "spheregeometry" << std::endl;
}

void twin::osimGeometryWrapper::implementEllipsoidGeometry(const SimTK::DecorativeEllipsoid &geom) {
    // const Transform X_GD = calcX_GD(geom);
    // const Vec3 radii = getScaleFactors(geom).elementwiseMultiply(geom.getRadii());
    // protocol.drawEllipsoid(X_GD, radii, getColor(geom), getRepresentation(geom),
    //        getResolution(geom));
    VLOG(4) << "ellipsoidgeometry" << std::endl;
}

void twin::osimGeometryWrapper::implementFrameGeometry(const SimTK::DecorativeFrame &geom) {
    // const Transform X_GD = calcX_GD(geom);
    // protocol.drawCoords(X_GD, geom.getAxisLength()*getScaleFactors(geom), getColor(geom));
    VLOG(4) << "framegeometry" << std::endl;
}

void twin::osimGeometryWrapper::implementTextGeometry(const SimTK::DecorativeText &geom) {
    // const Transform X_GD = calcX_GD(geom);
    //// The default is to face the camera.
    // bool faceCamera = geom.getFaceCamera()<0 ? true : (geom.getFaceCamera()!=0);
    // bool isScreenText = geom.getIsScreenText();
    // protocol.drawText(X_GD, getScaleFactors(geom), getColor(geom),
    //        geom.getText(), faceCamera, isScreenText);
    VLOG(4) << "textgeometry" << std::endl;
}

void twin::osimGeometryWrapper::implementMeshGeometry(const SimTK::DecorativeMesh &geom) {
    //
    // const Transform X_GD = calcX_GD(geom);
    // protocol.drawPolygonalMesh(geom.getMesh(), X_GD, getScaleFactors(geom),
    //        getColor(geom), getRepresentation(geom));
    VLOG(4) << "meshgeometry" << std::endl;
}


void twin::osimGeometryWrapper::implementMeshFileGeometry(const SimTK::DecorativeMeshFile &geom) {
    VLOG(4) << "Loading File: " << geom.getMeshFile() << std::endl;
    const PolygonalMesh &pMesh = geom.getMesh();
    const Transform X_GD = calcX_GD(geom);
    auto scale = getScaleFactors(geom);
    lastMesh.mesh = pMesh;
    lastMesh.scale = scale;
    lastMesh.trans = X_GD;
    lastMesh.color = geom.getColor();
}

Vec4 twin::osimGeometryWrapper::getColor(const DecorativeGeometry &geom, const Vec3 &defaultColor) {
    Vec4 result;
    if (geom.getColor()[0] >= 0)
        result.updSubVec<3>(0) = geom.getColor();
    else {
        const Vec3 def = defaultColor[0] >= 0 ? defaultColor : DefaultBodyColor;
        result.updSubVec<3>(0) = def;
    }
    result[3] = (geom.getOpacity() < 0 ? 1 : geom.getOpacity());
    return result;
}

int twin::osimGeometryWrapper::getRepresentation(const DecorativeGeometry &geom) const {
    if (geom.getRepresentation() == DecorativeGeometry::DrawDefault)
        return DecorativeGeometry::DrawSurface;
    return geom.getRepresentation();
}

unsigned short twin::osimGeometryWrapper::getResolution(const DecorativeGeometry &geom) const {
    if (geom.getResolution() <= 0)
        return 2;
    return std::max(static_cast<unsigned short>( 1), static_cast<unsigned short>( geom.getResolution() * 2));
}

Vec3 twin::osimGeometryWrapper::getScaleFactors(const DecorativeGeometry &geom) const {
    const Vec3 &scale = geom.getScaleFactors();
    Vec3 actual;
    for (int i = 0; i < 3; ++i)
        actual[i] = scale[i] <= 0 ? 1 : scale[i];
    return actual;
}
