#include "common.h"
#include "graphics.h"

int main() {
    graphics::initWindow(1920, 1080, true); 

    // Create an arbitrary node graph
    graphics::NodeGraph* root = new graphics::NodeGraph;
    root->children.resize(4);
    root->rel_pos   = vec2(400, 400);
    root->color     = graphics::palette[graphics::ColorPalette::PURPLE];
    root->color.a() = 0;

    for (int i = 0; i < 4; i++) {
        graphics::NodeGraph* node = new graphics::NodeGraph;
        node->rel_pos  = vec2(int(i / 2) * 250 + 20, (i % 2) * 250 + 20);
        node->rec_size = vec2(200, 200);
        node->color    = graphics::palette[i];
        node->parent   = root;
        root->children[i] = node;
    }
    root->rec_size = root->GetAABB().br + vec2(20, 20);

    // Main loop
    while (graphics::updateWindow(root)) {} // TODO: make it so that there is a start and end thing so i can put stuff here perhaps

    // Cleanup
    delete root;
    return 0;
}
