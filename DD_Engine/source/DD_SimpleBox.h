#pragma once
#include "framework.h"
#include "DD_GLHelper.h"
#include "DD_Transform.h"
#include "DD_Mesh.h"

class DD_SimpleBox : public DD_Mesh
{
public:
    static bool CachePipline();
    static void ClearPipline();
public:
    virtual void CreateMesh() override;
    virtual void PrepareForRender(const Matrix4& model, const Matrix4& view, const Matrix4& projection) override;
public:
    DD_SimpleBox();
    virtual ~DD_SimpleBox();

private:
    // shader program and uniform locations are static per mesh type
    static GLuint s_shaderProgram;
    static GLint s_modelLoc;
    static GLint s_viewLoc;
    static GLint s_projLoc;
    static bool s_initialized;
};