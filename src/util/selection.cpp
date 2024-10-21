#include "util/selection.h"

TWIN_NAMESPACE_BEGIN
    int RectangleSelection::counter = 0;

    RectangleSelection::RectangleSelection() {
        m_corners[0] = cppgl::vec2(-1, -1);
        m_corners[1] = cppgl::vec2(-1, -1);
        std::vector<cppgl::vec3> verts(4, cppgl::vec3(0, 0, 0));
        std::vector<uint> inds = {0, 1, 2, 0, 2, 3};
        if (!cppgl::Shader::valid("transparent_quad")) {
            cppgl::Shader("transparent_quad", "shader/transparent_quad.vert", "shader/transparent_quad.frag");
        }
        if (!cppgl::Material::valid("recsel_mat")) {
            cppgl::Material("recsel_mat");
        }
        cppgl::Geometry geo("recsel_geo_" + std::to_string(++counter), verts, inds);
        m_quad = cppgl::Drawelement("recsel_de_" + std::to_string(counter), cppgl::Shader::find("transparent_quad"), cppgl::Mesh("recsel_mesh_" + std::to_string(counter), geo, cppgl::Material::find("recsel_mat"), GL_DYNAMIC_DRAW));
    }

    void RectangleSelection::ResetSelection() {
        m_corners[0] = cppgl::vec2(-1, -1);
        m_corners[1] = cppgl::vec2(-1, -1);
        static constexpr float verts[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        m_quad->mesh->update_vertex_buffer(0, &verts);
    }

    bool Intersect(cppgl::vec4 region, std::vector<cppgl::vec2> tri) {
        // Performs sat intersect test
        auto which_side = [](std::vector<cppgl::vec2> poly, const cppgl::vec2 &d, const cppgl::vec2 &p) {
            int positive = 0, negative = 0;
            for (unsigned long int i = 0; i < poly.size(); i++) {
                float t = (poly.at(i) - p).dot(d);
                if (t > 0) {
                    positive++;
                } else if (t < 0) {
                    negative++;
                }
                if (positive && negative) {
                    return 0;
                }
            }
            return positive ? +1 : -1;
        };
        std::vector<cppgl::vec2> box = {{region[0], region[2]},
                                        {region[1], region[2]},
                                        {region[1], region[3]},
                                        {region[0], region[3]}};
        for (unsigned long int i0 = 0, i1 = box.size() - 1; i0 != box.size(); i1 = i0, ++i0) {
            cppgl::vec2 dir = box.at(i1) - box.at(i0);
            cppgl::vec2 d(dir[1], -dir[0]);

            int w = which_side(tri, d, box.at(i0));
            if (w > 0) {
                return false;
            }
        }
        for (unsigned long int i0 = 0, i1 = tri.size() - 1; i0 != tri.size(); i1 = i0, ++i0) {
            cppgl::vec2 dir = tri.at(i0) - tri.at(i1);
            cppgl::vec2 d(dir[1], -dir[0]);
            int w = which_side(box, d, tri.at(i0));
            if (w > 0) {
                return false;
            }
        }
        return true;
    }

    std::vector<int> RectangleSelection::Select(const cppgl::Drawelement &mesh, const Ray &ray, const cppgl::vec2 &pixel_percent) {
        if (m_corners[0][0] == -1) {
            m_corners[0] = pixel_percent;
        } else {
            m_corners[1] = pixel_percent;
        }

        if (m_corners[1][0] == -1) {
            return std::vector<int>();
        }
        std::vector<int> ret;

        auto ordered = [](float a, float b) {
            if (a < b)
                return std::make_pair(a, b);
            else
                return std::make_pair(b, a);
        };
        auto [left, right] = ordered(m_corners[0][0], m_corners[1][0]);
        auto [top, bottom] = ordered(m_corners[0][1], m_corners[1][1]);

        float verts[12] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
        verts[0] = -(1 - 2 * left);
        verts[1] = -(1 - 2 * (1 - bottom));
        verts[3] = -(1 - 2 * right);
        verts[4] = -(1 - 2 * (1 - bottom));
        verts[6] = -(1 - 2 * right);
        verts[7] = -(1 - 2 * (1 - top));
        verts[9] = -(1 - 2 * left);
        verts[10] = -(1 - 2 * (1 - top));
        m_quad->mesh->update_vertex_buffer(0, &verts);

        Frustum tmp;
        tmp.computePlanesFromMatrices(cppgl::current_camera()->proj, cppgl::current_camera()->view);
        m_frustum = tmp.SubFrustum(left, right, top, bottom);
        cppgl::mat4 viewProj = cppgl::current_camera()->proj * cppgl::current_camera()->view;
        for (int i = 0; i != static_cast<int>(mesh->mesh->num_indices / 3); ++i) {

            cppgl::ivec3 tri(static_cast<int>(mesh->mesh->geometry->get_index(i * 3)), static_cast<int>(mesh->mesh->geometry->get_index(i * 3 + 1)), static_cast<int>(mesh->mesh->geometry->get_index(i * 3 + 2)));

            float l = left * 2 - 1;
            float r = right * 2 - 1;
            float t = top * 2 - 1;
            float b = bottom * 2 - 1;

            cppgl::vec4 vs[] = {viewProj * cppgl::make_vec4(mesh->mesh->geometry->get_position(tri[0]), 1), viewProj * cppgl::make_vec4(mesh->mesh->geometry->get_position(tri[1]), 1), viewProj * cppgl::make_vec4(mesh->mesh->geometry->get_position(tri[2]), 1)};
            for (cppgl::vec4 &v: vs) {
                v = v / v[3];
            }

            if (Intersect({l, r, -t, -b}, {{vs[0][0], vs[0][1]},
                                           {vs[1][0], vs[1][1]},
                                           {vs[2][0], vs[2][1]}})) {
                ret.push_back(i);
            }
        }
        return ret;
    }

    void RectangleSelection::Render() const {
        if (!m_quad)return;
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glDisable(GL_DEPTH_TEST);
        glDepthMask(GL_FALSE);
        m_quad->bind();
        m_quad->draw();
        m_quad->unbind();
        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);
        glEnable(GL_DEPTH_TEST);
    }


TWIN_NAMESPACE_END

