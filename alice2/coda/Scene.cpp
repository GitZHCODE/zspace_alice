#include "Scene.h"
#include "Shaders.h"

namespace {
    void GetModelMatrix(float* matrix) {
        // Identity matrix
        matrix[0] = 1.0f; matrix[4] = 0.0f; matrix[8]  = 0.0f; matrix[12] = 0.0f;
        matrix[1] = 0.0f; matrix[5] = 1.0f; matrix[9]  = 0.0f; matrix[13] = 0.0f;
        matrix[2] = 0.0f; matrix[6] = 0.0f; matrix[10] = 1.0f; matrix[14] = 0.0f;
        matrix[3] = 0.0f; matrix[7] = 0.0f; matrix[11] = 0.0f; matrix[15] = 1.0f;
    }

    void MultiplyMatrices(const float* a, const float* b, float* result) {
        for (int i = 0; i < 4; ++i) {
            for (int j = 0; j < 4; ++j) {
                float sum = 0.0f;
                for (int k = 0; k < 4; ++k) {
                    sum += a[i + k * 4] * b[k + j * 4];
                }
                result[i + j * 4] = sum;
            }
        }
    }
}

namespace coda {

bool Scene::Initialize(int width, int height) {
    printf("Scene initialization started\n");
    
    auto& renderer = Renderer::Get();
    if (!renderer.Initialize(width, height)) {
        return false;
    }

    if (!InitializeShaders()) {
        return false;
    }

    SetBackgroundColor(0.85f);
    m_Camera.SetAspect(float(width) / float(height));
    
    printf("Scene initialization completed\n");
    return true;
}

void Scene::Cleanup() {
    Clear();
}

bool Scene::InitializeShaders() {
    auto& renderer = Renderer::Get();
    
    // Create lit shader program
    m_LitShader.id = glCreateProgram();
    
    // Create and compile vertex shader
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &shaders::litShader.vertex, nullptr);
    glCompileShader(vertexShader);
    
    // Create and compile fragment shader
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &shaders::litShader.fragment, nullptr);
    glCompileShader(fragmentShader);
    
    // Attach shaders
    glAttachShader(m_LitShader.id, vertexShader);
    glAttachShader(m_LitShader.id, fragmentShader);
    
    // Bind attribute locations before linking
    glBindAttribLocation(m_LitShader.id, 0, "aPosition");
    glBindAttribLocation(m_LitShader.id, 1, "aNormal");
    
    // Link program
    glLinkProgram(m_LitShader.id);
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (!m_LitShader.id) {
        printf("Failed to create lit shader program\n");
        return false;
    }
    m_LitShader.attributes.position = 0;
    m_LitShader.attributes.normal = 1;

    // Create simple shader program
    m_SimpleShader.id = glCreateProgram();
    
    // Create and compile vertex shader
    vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &shaders::simpleShader.vertex, nullptr);
    glCompileShader(vertexShader);
    
    // Create and compile fragment shader
    fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &shaders::simpleShader.fragment, nullptr);
    glCompileShader(fragmentShader);
    
    // Attach shaders
    glAttachShader(m_SimpleShader.id, vertexShader);
    glAttachShader(m_SimpleShader.id, fragmentShader);
    
    // Bind attribute location before linking
    glBindAttribLocation(m_SimpleShader.id, 0, "aPosition");
    
    // Link program
    glLinkProgram(m_SimpleShader.id);
    
    // Clean up shaders
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    
    if (!m_SimpleShader.id) {
        printf("Failed to create simple shader program\n");
        return false;
    }
    m_SimpleShader.attributes.position = 0;

    printf("Shader programs created successfully: Lit=%u, Simple=%u\n", m_LitShader.id, m_SimpleShader.id);

    return true;
}

void Scene::Render() {
    auto& renderer = Renderer::Get();
    renderer.Clear();

    // Calculate matrices once
    float view[16], proj[16], model[16];
    m_Camera.GetViewMatrix(view);
    m_Camera.GetProjectionMatrix(proj);
    GetModelMatrix(model);

    // Store matrices for immediate mode drawing
    memcpy(m_CurrentView, view, sizeof(m_CurrentView));
    memcpy(m_CurrentProj, proj, sizeof(m_CurrentProj));
    memcpy(m_CurrentModel, model, sizeof(m_CurrentModel));
    
    // Render non-lit objects with simple shader
    renderer.UseShaderProgram(m_SimpleShader.id);
    renderer.SetMatrix4f("uViewMatrix", view);
    renderer.SetMatrix4f("uProjectionMatrix", proj);
    renderer.SetMatrix4f("uModelMatrix", model);
    renderer.SetFloat("uPointSize", 10.0f);

    // Draw coordinate axes
    DrawPoint(Vec3f(0.0f, 0.0f, 0.0f), Color::Red(), 10.0f);
    DrawLine(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(1.0f, 0.0f, 0.0f), Color::Red());
    DrawLine(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 1.0f, 0.0f), Color::Green());
    DrawLine(Vec3f(0.0f, 0.0f, 0.0f), Vec3f(0.0f, 0.0f, 1.0f), Color::Blue());
    
    // Render lit objects (mesh faces)
    renderer.UseShaderProgram(m_LitShader.id);
    renderer.SetMatrix4f("uViewMatrix", view);
    renderer.SetMatrix4f("uProjectionMatrix", proj);
    renderer.SetMatrix4f("uModelMatrix", model);
    
    // Set up lighting
    Vec3f directionalLight = Vec3f(1.0f, 1.0f, 1.0f).Normalize() * 0.95f;
    Vec3f ambientLight(0.75f, 0.75f, 0.75f);
    
    renderer.SetVector3f("uDirectionalLight", directionalLight.x, directionalLight.y, directionalLight.z);
    renderer.SetVector3f("uAmbientLight", ambientLight.x, ambientLight.y, ambientLight.z);

    // Draw mesh faces only
    for (const auto* obj : m_Objects) {
        if (auto* mesh = dynamic_cast<const ObjMesh*>(obj)) {
            mesh->DrawFaces();
        }
    }

    // Render non-lit objects with simple shader
    renderer.UseShaderProgram(m_SimpleShader.id);
    renderer.SetMatrix4f("uViewMatrix", view);
    renderer.SetMatrix4f("uProjectionMatrix", proj);
    renderer.SetMatrix4f("uModelMatrix", model);
    renderer.SetFloat("uPointSize", 10.0f);

    // Draw graph objects and mesh vertices/edges with simple shader
    for (const auto* obj : m_Objects) {
        if (auto* graph = dynamic_cast<const ObjGraph*>(obj)) {
            graph->Draw();
        }
        if (auto* mesh = dynamic_cast<const ObjMesh*>(obj)) {
            mesh->DrawVerticesAndEdges();
        }
    }

    renderer.DisableVertexAttribArray(0);
    renderer.DisableVertexAttribArray(1);
}

void Scene::AddObject(Obj* obj) {
    if (obj) {
        m_Objects.push_back(obj);
    }
}

void Scene::Clear() {
    m_Objects.clear();
}

void Scene::DrawPoint(const Vec3f& pos, const Color& color, float size) {
    auto& renderer = Renderer::Get();
    
    renderer.UseShaderProgram(m_SimpleShader.id);
    
    renderer.SetMatrix4f("uViewMatrix", m_CurrentView);
    renderer.SetMatrix4f("uProjectionMatrix", m_CurrentProj);
    renderer.SetMatrix4f("uModelMatrix", m_CurrentModel);
    
    float vertices[] = { pos.x, pos.y, pos.z };
    renderer.UpdateVertexBuffer(vertices, sizeof(vertices));
    
    renderer.SetVector4f("uColor", color.r, color.g, color.b, color.a);
    renderer.SetFloat("uPointSize", size);
    renderer.SetVertexAttribute(0, 3, 0, 0);
    
    renderer.DrawArrays(GL_POINTS, 0, 1);
}

void Scene::DrawLine(const Vec3f& p0, const Vec3f& p1, const Color& color, float weight) {
    auto& renderer = Renderer::Get();
    
    renderer.UseShaderProgram(m_SimpleShader.id);
    
    renderer.SetMatrix4f("uViewMatrix", m_CurrentView);
    renderer.SetMatrix4f("uProjectionMatrix", m_CurrentProj);
    renderer.SetMatrix4f("uModelMatrix", m_CurrentModel);
    
    float vertices[] = {
        p0.x, p0.y, p0.z,
        p1.x, p1.y, p1.z
    };
    
    renderer.UpdateVertexBuffer(vertices, sizeof(vertices));
    
    renderer.SetVector4f("uColor", color.r, color.g, color.b, color.a);
    renderer.SetVertexAttribute(0, 3, 0, 0);
    
    glLineWidth(weight);
    renderer.DrawArrays(GL_LINES, 0, 2);
    glLineWidth(1.0f);
}

} // namespace coda 