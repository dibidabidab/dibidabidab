
#ifndef GAME_LIGHTMAPRENDERER_H
#define GAME_LIGHTMAPRENDERER_H


#include <graphics/frame_buffer.h>
#include <graphics/camera.h>
#include <graphics/shader_asset.h>
#include <graphics/3d/mesh.h>
#include <graphics/3d/vert_buffer.h>
#include "../../../level/room/Room.h"

class LightMapRenderer
{
    Room *room;

    ShaderAsset shader;

    SharedMesh quadMesh;
    VertData lightsData;
    int lightsDataBuffer = -1;

  public:

    FrameBuffer *fbo = NULL;

    LightMapRenderer(Room *room);

    void render(const Camera &cam, const SharedTexture &shadowTexture);

    ~LightMapRenderer();

};


#endif
