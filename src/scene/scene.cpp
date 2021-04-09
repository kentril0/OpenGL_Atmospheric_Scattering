
#include "gui.hpp"

#include <glad/glad.h>
#include <glm/gtc/matrix_transform.hpp>

#define QUAD_VERTICES 6
#define INSTANCED true

namespace GUI {

Renderer* Renderer::instance;

Renderer::Renderer()
  : dirty_buffers(true)
{
    // Each widget is a quad
    const float vertices[] {
      // position, texture coordinates TODO
        0.0f, 1.0f, // 0.0f, 0.0f,
        0.0f, 0.0f, // 0.0f, 1.0f,
        1.0f, 0.0f, // 1.0f, 1.0f,

        0.0f, 1.0f, // 0.0f, 0.0f,
        1.0f, 0.0f, // 1.0f, 1.0f,
        1.0f, 1.0f//,  1.0f, 0.0f
    };

    // Setup VBOs for positions, models and colors
    std::shared_ptr<VertexBuffer> vbo = 
        std::make_shared<VertexBuffer>(sizeof(vertices), vertices);

    vbo->set_layout(BufferLayout({
        { ElementType::Float2, "Position"}
        // {ElementType::Float2, "Texture"} TODO
    }));

    // Will be allocated later with new widgets
    vbo_models = std::make_shared<VertexBuffer>();
    vbo_colors = std::make_shared<VertexBuffer>();

    vbo_models->set_layout(BufferLayout({ { ElementType::Mat4, "Models"} }));
    vbo_colors->set_layout(BufferLayout({ { ElementType::Float3, "Colors"} }));

    vao.add_vertex_buffer(vbo);
    vao.add_vertex_buffer(vbo_colors, INSTANCED);
    vao.add_vertex_buffer(vbo_models, INSTANCED);   // mat4 instanced anyways
}

void Renderer::irender()
{
    if (dirty_buffers)
    {
        LOG_INFO("Drawing: " << models.size() << " buffers? " << dirty_buffers);
        setup_buffers();
    }

    // glActiveTexture(GL_TEXTURE0);
    // texture.bind();

    shader.use();
    shader.set_mat4("proj", proj);

    vao.bind();

    glDrawArraysInstanced(GL_TRIANGLES, 0, QUAD_VERTICES, models.size());

    // Draw text
    TextRenderer::render_fmts();
}

Widget* Renderer::iadd_widget(const Widget& w)
{
    widgets.push_back(w);
    Widget* wr = &widgets.back();

    // Screen dimensions as parent's are passed
    wr->apply_constraints(glm::vec2(0,0), glm::vec2(screen.x, screen.y));

    return wr;
}

void Renderer::irescale(size_t width, size_t height)
{
    set_proj_mat(width, height);
    // TODO more
    set_screen_dimensions(width, height);
}

void Renderer::setup_buffers()
{
    if (!dirty_buffers)
        return;

    assert(models.size() == colors.size());

    // Reallocate VBOs for models and colors
    vbo_models->reallocate(sizeof(glm::mat4) * models.size(), models.data());
    vbo_colors->reallocate(sizeof(glm::vec3) * colors.size(), colors.data());

    dirty_buffers = false;
}

uint32_t Renderer::ienqueue(const glm::mat4& model, const glm::vec3& color)
{
    massert(models.size() == colors.size(), "GUI::Renderer queue sizes do not match!");
    uint32_t index = models.size();
    
    models.emplace_back(model);
    colors.emplace_back(color);
    dirty_buffers = true;

    return index;
}

void Renderer::ienqueue_at(uint32_t index, const glm::mat4& model)
{
    assert(models.size() > index);
    models[index] = model;
    dirty_buffers = true;
}

void Renderer::ienqueue_at(uint32_t index, const glm::vec3& color)
{
    assert(colors.size() > index);
    colors[index] = color;
    dirty_buffers = true;
}

void Renderer::set_proj_mat(size_t screen_width, size_t screen_height)
{
    proj = glm::ortho(0.0f, static_cast<float>(screen_width),
                      0.0f, static_cast<float>(screen_height));
}

void Renderer::idequeue(uint32_t index)
{
    massert(models.size() == colors.size(), "GUI::Renderer queue sizes do not match!");
    assert(models.size() > index);

    models.erase(models.begin() + index);
    colors.erase(colors.begin() + index);
    dirty_buffers = true;
}

// --------------------------------------------------------------------------------
// VerticalLayout 
// --------------------------------------------------------------------------------

/**
 * @brief Recalculate both positions and sizes of widgets relative to their 
 *        parent. First, widgets with size constraints (sized widgets) take up parent's
 *        height, theni the left size is proprtionally devided among the other widgets
 *        (expanding widgets, that take full parent's width).
 */
void VerticalLayout::fit_elements()
{
    LOG_INFO("Fitting " << elements.size() << " elements");
    assert(parent != nullptr);

    const float parent_height = parent->size.y;
    const float parent_width = parent->size.x;
    const uint32_t total_widgets = elements.size();
    
    // Number of widgets without size constraints (so-called expanding widgets), 
    //  these will have the same height 'element_height' proportional to the left space
    uint32_t expanding_widgets = 0;
    float left_height = parent_height;

    std::vector<bool> sized_widgets(total_widgets, false);

    for (uint32_t i = 0; i < total_widgets; ++i)
    {
        Widget* w = &elements[i];

        // Translate to parent's position and 'clamp' size to parent's
        w->pos += parent->pos;
        w->size = parent->size;

        // Widgets with relative size constraints
        if (w->apply_constraints(parent->pos, parent->size, 
              { ConstraintType::REL_SIZE, //ConstraintType::REL_WIDTH,
                ConstraintType::REL_HEIGHT}))
        {
            sized_widgets[i] = true;
            left_height -= w->size.y;
        }
        else
        {
            sized_widgets[i] = false;
            expanding_widgets++;
        }
    }

    // Calculate the height of expanding widgets
    const float element_height = left_height / expanding_widgets;
    float step = parent_height;

    // Adjust height of elements according to their type (expandable x sized)
    for (uint32_t i = 0; i < total_widgets; ++i)
    {
        Widget* w = &elements[i];

        w->size.y = sized_widgets.at(i) ? w->size.y : element_height;
        step -= w->size.y;

        w->pos.y += step;

        // Non-sized constraints that change the position relative to the parent
        w->apply_constraints(w->pos, glm::vec2(parent_width, w->size.y));

        //LOG_INFO("W.pos: " << w->pos.x << " , " << w->pos.y);
        ///LOG_INFO("W.size: " << w->size.x << " x " << w->size.y);

        // Update its model matrix
        w->update_model();
    }
}

// --------------------------------------------------------------------------------
// ScreenConstraint 
// --------------------------------------------------------------------------------
void ScreenConstraint::apply(glm::vec2* pos, glm::vec2* size, 
                             const glm::vec2& rel_pos,
                             const glm::vec2& rel_size) const
{
    switch(type)
    {
        case ConstraintType::NONE:
            return;
        case ConstraintType::REL_POS:
            *pos = glm::vec2(rel_pos.x * factors.x, rel_pos.y * factors.y);
            return;
        case ConstraintType::REL_POS_X:
            pos->x = rel_pos.x * factors.x;
            return;
        case ConstraintType::REL_POS_Y:
            pos->y = rel_pos.y * factors.x;
            return;
        case ConstraintType::REL_SIZE:
            *size = glm::vec2(rel_size.x * factors.x, rel_size.y * factors.y);
            return;
        case ConstraintType::REL_WIDTH:
            size->x = rel_size.x * factors.x;
            return;
        case ConstraintType::REL_HEIGHT:
            size->y = rel_size.y * factors.x;
            return;
        case ConstraintType::CENTER:
            pos->x = rel_pos.x + rel_size.x * 0.5f - size->x * 0.5f;
            pos->y = rel_pos.y + rel_size.y * 0.5f - size->y * 0.5f;
            return;
        case ConstraintType::TOP_LEFT:
            pos->x = rel_pos.x;
            pos->y = rel_pos.y + rel_size.y - size->y;
            return;
        case ConstraintType::TOP_RIGHT:
            pos->x = rel_pos.x + rel_size.x - size->x;
            pos->y = rel_pos.y + rel_size.y - size->y;
            return;
        case ConstraintType::BOTTOM_LEFT:
            *pos = rel_pos;
            return;
        case ConstraintType::BOTTOM_RIGHT:
            pos->x = rel_pos.x + rel_size.x - size->x;
            pos->y = rel_pos.y;
            return;
        case ConstraintType::TOP_CENTER:
            pos->x = rel_pos.x + rel_size.x * 0.5f - size->x * 0.5f;
            pos->y = rel_pos.y + rel_size.y - size->y;
            return;
        case ConstraintType::BOTTOM_CENTER:
            pos->x = rel_pos.x + rel_size.x * 0.5f - size->x * 0.5f;
            pos->y = rel_pos.y;
            return;
    }
    massert(false, "Unknown Screen Constraint type!");
    return;
};

float ScreenConstraint::clamp(float v)
{
    return glm::clamp(v, 0.0f, 1.0f);
}

// --------------------------------------------------------------------------------
// Widget
// --------------------------------------------------------------------------------

Widget::Widget(const std::vector<FlagType>& flags, 
               const std::set<ScreenConstraint>& sc, 
               const glm::vec3& color)
  : pos(0.0f, 0.0f), size(1.0f, 1.0f), constraints(sc)
{
    LOG_INFO("Widget CONSTR");
    for (auto& flag : flags)
    {
        switch(flag.type)
        {
            case Flag::Position:
                pos = flag.val;
                break;
            case Flag::Size:
                size = flag.val;
                break;
        }
    }
    
    // Get the index to a render queue for later changes in model matrix or color
    queue_index = Renderer::enqueue(create_model_mat(), color);
}

Widget::Widget(const std::set<ScreenConstraint>& sc, 
               const glm::vec3& color)
  : pos(0.0f, 0.0f), size(1.0f, 1.0f), constraints(sc)
{
    LOG_INFO("Widget CONSTR");
    
    // Get the index to a render queue for later changes in model matrix or color
    queue_index = Renderer::enqueue(create_model_mat(), color);
}

Widget::Widget(const glm::vec3& color)
  : pos(0.0f, 0.0f), size(1.0f, 1.0f)
{
    LOG_INFO("Widget CONSTR");
    // Get the index to a render queue for later changes in model matrix or color
    queue_index = Renderer::enqueue(create_model_mat(), color);
}

void Widget::update_model()
{
    LOG_INFO("UPDATE MODELMATIRX"); 
    Renderer::enqueue_model_at(queue_index, create_model_mat());
}

glm::mat4 Widget::create_model_mat()
{
    glm::mat4 model(1.0f);

    // Advised order: scale, (rotate), translate.
    model = glm::translate(model, glm::vec3(pos, 0.0f));
    model = glm::scale(model, glm::vec3(size, 1.0f));

    return model;
}

void Widget::apply_constraints(const glm::vec2& rel_pos, const glm::vec2& rel_size)
{
    for (auto c = constraints.begin(); c != constraints.end(); ++c)
        c->apply(&pos, &size, rel_pos, rel_size);

    LOG_INFO("WIDGET: [" << pos.x << "," << pos.y << "] " 
              << size.x << " x " << size.y);
    update_model();
}   

bool Widget::apply_constraints(const glm::vec2& rel_pos, const glm::vec2& rel_size,
                       const std::unordered_set<ConstraintType>& types)
{
    bool applied = false;
    for (auto c = constraints.begin(); c != constraints.end(); ++c)
        if (types.count(c->get_type()))
        {
            applied = true;
            c->apply(&pos, &size, rel_pos, rel_size);
            constraints.erase(c);
        }   

    update_model();
    return applied;
}

/*
Widget::~Widget()
{
    LOG_INFO("Widget destroyed");
}
*/

// --------------------------------------------------------------------------------
// Label 
// --------------------------------------------------------------------------------
Label::Label(const char* txt, 
      uint32_t font_size, 
      const glm::vec3& color,
      const std::vector<FlagType>& flags,
      const std::set<ScreenConstraint>& sc,
      const glm::vec3& bgcolor)
    : Widget(flags, sc, bgcolor), text(txt), size(font_size), color(color)
{
    // Preload font with size if not loaded already
    TextRenderer::set_font_size(size);

    text_queue = TextRenderer::enqueue(TextRenderer::Fmt(&text, size, pos, color));
    LOG_INFO("Label queued at: " << text_queue << text);
}

/*
Label::~Label()
{
    LOG_INFO("Label destroyed");
}
*/


};  // Namespace GUI

