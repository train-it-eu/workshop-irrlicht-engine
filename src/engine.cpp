/*
 * Copyright (c) 2019, Train IT
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

/*
 * This workshop is based on Irrlicht Tutorial 7: Collision
 * (http://irrlicht.sourceforge.net/docu/example007.html)
 */

#include <irrlicht-engine/engine.h>
#include <cassert>
#include <stdexcept>
#include <string>

namespace {

enum { id_flag_not_pickable = 0, id_flag_is_pickable = 1 << 0, id_flag_is_highlightable = 1 << 1 };

const std::wstring workshop_title = L"Modern C++ Design - Part I";

}  // namespace

/* ********************************* S E L E C T O R ********************************* */

namespace {

irr::scene::ITriangleSelector* init_selector(workshop::engine::irr_runtime* r, irr::scene::IAnimatedMeshSceneNode* o)
{
  assert(r);
  assert(r->smgr);
  assert(o);

  irr::scene::ITriangleSelector* resource = r->smgr->createTriangleSelector(o);
  assert(resource);

  return resource;
}

}  // namespace

workshop::selector::selector(engine* e, object_handle* object)
{
  assert(e);
  assert(object);

  resource_ = init_selector(e->runtime(), object->resource_);
}

workshop::selector::~selector()
{
  assert(resource_);
  resource_->drop();
}

/* ********************************* O B J E C T ********************************* */

namespace {

irr::scene::IAnimatedMeshSceneNode* init_object_handle(workshop::engine::irr_runtime* r,
                                                       workshop::object_handle::type type, const std::string& name,
                                                       const std::string& irrlicht_media_path)
{
  assert(r);
  assert(r->smgr);
  assert(r->driver);

  irr::scene::IAnimatedMeshSceneNode* resource = nullptr;

  switch (type) {
    case workshop::object_handle::type_faerie: {
      // add an MD2 node, which uses vertex-based animation
      irr::scene::IAnimatedMesh* mesh = r->smgr->getMesh((irrlicht_media_path + "/faerie.md2").c_str());
      if (!mesh) throw std::runtime_error("Cannot open mesh '" + irrlicht_media_path + "/faerie.md2'");
      resource = r->smgr->addAnimatedMeshSceneNode(mesh, 0, id_flag_is_pickable | id_flag_is_highlightable);
      assert(resource);
      resource->setScale(irr::core::vector3df(1.6f));
      resource->setMD2Animation(irr::scene::EMAT_POINT);
      resource->setAnimationSpeed(20.f);
      irr::video::ITexture* tex = r->driver->getTexture((irrlicht_media_path + "/faerie2.bmp").c_str());
      if (!tex) throw std::runtime_error("Cannot open texture '" + irrlicht_media_path + "/faerie2.bmp'");
      irr::video::SMaterial material;
      material.setTexture(0, tex);
      material.Lighting = true;
      material.NormalizeNormals = true;
      resource->getMaterial(0) = material;
      resource->setName(name.c_str());
    } break;

    case workshop::object_handle::type_ninja: {
      // this B3D file uses skinned skeletal animation
      irr::scene::IAnimatedMesh* mesh = r->smgr->getMesh((irrlicht_media_path + "/ninja.b3d").c_str());
      if (!mesh) throw std::runtime_error("Cannot open mesh '" + irrlicht_media_path + "/ninja.b3d'");
      resource = r->smgr->addAnimatedMeshSceneNode(mesh, 0, id_flag_is_pickable | id_flag_is_highlightable);
      assert(resource);
      resource->setScale(irr::core::vector3df(10));
      resource->setAnimationSpeed(8.f);
      resource->getMaterial(0).NormalizeNormals = true;
      resource->getMaterial(0).Lighting = true;
      resource->setName(name.c_str());
    } break;

    case workshop::object_handle::type_dwarf: {
      // this X file uses skeletal animation, but without skinning
      irr::scene::IAnimatedMesh* mesh = r->smgr->getMesh((irrlicht_media_path + "/dwarf.x").c_str());
      if (!mesh) throw std::runtime_error("Cannot open mesh '" + irrlicht_media_path + "/dwarf.x'");
      resource = r->smgr->addAnimatedMeshSceneNode(mesh, 0, id_flag_is_pickable | id_flag_is_highlightable);
      assert(resource);
      resource->setAnimationSpeed(20.f);
      resource->getMaterial(0).Lighting = true;
      resource->setName(name.c_str());
    } break;

    case workshop::object_handle::type_yodan: {
      // this mdl file uses skinned skeletal animation
      irr::scene::IAnimatedMesh* mesh = r->smgr->getMesh((irrlicht_media_path + "/yodan.mdl").c_str());
      if (!mesh) throw std::runtime_error("Cannot open mesh '" + irrlicht_media_path + "/yodan.mdl'");
      resource = r->smgr->addAnimatedMeshSceneNode(mesh, 0, id_flag_is_pickable | id_flag_is_highlightable);
      assert(resource);
      resource->setScale(irr::core::vector3df(0.8f));
      resource->getMaterial(0).Lighting = true;
      resource->setAnimationSpeed(20.f);
      resource->setName(name.c_str());
    } break;

    case workshop::object_handle::type_unknown:
      // nothing to do
      break;

    default:
      assert(0);
  }

  return resource;
}

}  // namespace

workshop::object_handle::object_handle(engine* e, type t, const std::string* name) : type_(t), name_(name)
{
  assert(e);
  assert(name);

  resource_ = init_object_handle(e->runtime(), type_, *name_, e->irrlicht_media_path());
}

workshop::object_handle::object_handle(irr::scene::IAnimatedMeshSceneNode* resource) : resource_(resource)
{
  assert(resource_);
}

void workshop::object_handle::position(float x, float y, float z)
{
  assert(resource_);

  resource_->setPosition(irr::core::vector3df(x, y, z));
}

void workshop::object_handle::rotation(float x, float y, float z)
{
  assert(resource_);
  assert(-180 <= x && x <= 180);
  assert(-180 <= y && y <= 180);
  assert(-180 <= z && z <= 180);

  resource_->setRotation(irr::core::vector3df(x, y, z));
}

void workshop::object_handle::selector(workshop::selector* s)
{
  assert(resource_);
  assert(s);
  assert(s->resource_);

  resource_->setTriangleSelector(s->resource_);
}

void workshop::object_handle::highlight(bool select)
{
  assert(resource_);

  resource_->setMaterialFlag(irr::video::EMF_LIGHTING, !select);
}

void workshop::object_handle::name(std::string* name) const
{
  assert(name);

  *name = resource_->getName();
}

/* ********************************* C A M E R A ********************************* */

namespace {

irr::scene::ICameraSceneNode* init_camera(irr::scene::ISceneManager* smgr, irr::scene::IMeshSceneNode* level)
{
  assert(smgr);
  assert(level);

  irr::scene::ICameraSceneNode* resource =
    smgr->addCameraSceneNodeFPS(0, 50.0f, .3f, id_flag_not_pickable, 0, 0, true, 2.f);
  if (!resource) throw std::runtime_error("Cannot add camera scene node");

  irr::scene::ISceneNodeAnimator* anim =
    smgr->createCollisionResponseAnimator(level->getTriangleSelector(), resource, irr::core::vector3df(30, 50, 30),
                                          irr::core::vector3df(0, -10, 0), irr::core::vector3df(0, 30, 0));
  if (!anim)
    throw std::runtime_error("Cannot create scene node animator for doing automatic collision detection and response");

  resource->addAnimator(anim);
  anim->drop();

  return resource;
}

}  // namespace

workshop::camera::camera(irr::scene::ISceneManager* smgr, irr::scene::IMeshSceneNode* level) :
    resource_(init_camera(smgr, level))
{
}

void workshop::camera::position(float x, float y, float z)
{
  assert(resource_);

  resource_->setPosition(irr::core::vector3df(x, y, z));
}

void workshop::camera::target(float x, float y, float z)
{
  assert(resource_);

  resource_->setTarget(irr::core::vector3df(x, y, z));
}

/* ********************************* E V E N T   R E C E I V E R ********************************* */

bool workshop::engine::event_receiver::OnEvent(const irr::SEvent& event)
{
  // Remember whether each key is down or up
  if (event.EventType == irr::EET_KEY_INPUT_EVENT)
    if (event.KeyInput.PressedDown && event.KeyInput.Key == irr::KEY_KEY_Q) quit_ = true;
  return false;
}

/* ********************************* 3 D   E N G I N E ********************************* */

namespace {

irr::video::E_DRIVER_TYPE convert(workshop::engine::device_type type)
{
  irr::video::E_DRIVER_TYPE irr_type[] = {irr::video::EDT_NULL, irr::video::EDT_SOFTWARE, irr::video::EDT_DIRECT3D9,
                                          irr::video::EDT_OPENGL};
  assert(sizeof(irr_type) / sizeof(irr_type[0]) == workshop::engine::device_num);
  return irr_type[type];
}

irr::IrrlichtDevice* init_device(const std::string& irrlicht_media_path, irr::u32 width, irr::u32 height, irr::u32 bpp,
                                 bool full_screen, bool stencil, bool vsync, workshop::engine::device_type device_type,
                                 workshop::engine::event_receiver* event_receiver)
{
  assert(event_receiver);

  // create Irrlicht device - the most important object of the engine
  irr::IrrlichtDevice* device = irr::createDevice(convert(device_type), irr::core::dimension2d<irr::u32>(width, height),
                                                  bpp, full_screen, stencil, vsync, event_receiver);
  if (!device) throw std::runtime_error("Failed to create a device");

  // add Quake 3 map resources to Irrlicht local file system
  if (!device->getFileSystem()->addFileArchive((irrlicht_media_path + "/map-20kdm2.pk3").c_str())) {
    device->drop();
    throw std::runtime_error("Cannot load archive '" + irrlicht_media_path + "/map-20kdm2.pk3'");
  }

  device->setWindowCaption(workshop_title.c_str());

  // get rid of the mouse cursor.  We'll use a billboard to show what we're looking at.
  device->getCursorControl()->setVisible(false);

  return device;
}

irr::gui::IGUIFont* init_font(irr::gui::IGUIEnvironment* guienv, const std::string& irrlicht_media_path)
{
  assert(guienv);

  irr::gui::IGUIFont* font = guienv->getFont((irrlicht_media_path + "/fonthaettenschweiler.bmp").c_str());
  if (!font) throw std::runtime_error("Cannot load font '" + irrlicht_media_path + "/fonthaettenschweiler.bmp'");
  return font;
}

irr::scene::IBillboardSceneNode* init_laser(irr::scene::ISceneManager* smgr, irr::video::IVideoDriver* driver,
                                            const std::string& irrlicht_media_path)
{
  assert(smgr);
  assert(driver);

  // add the laser
  irr::scene::IBillboardSceneNode* laser = smgr->addBillboardSceneNode();
  if (!laser) throw std::runtime_error("Cannot create a laser node");

  irr::video::ITexture* laser_tex = driver->getTexture((irrlicht_media_path + "/particle.bmp").c_str());
  if (!laser_tex) throw std::runtime_error("Cannot open texture '" + irrlicht_media_path + "/particle.bmp'");

  // init laser
  laser->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
  laser->setMaterialTexture(0, laser_tex);
  laser->setMaterialFlag(irr::video::EMF_LIGHTING, false);
  laser->setMaterialFlag(irr::video::EMF_ZBUFFER, false);
  laser->setSize(irr::core::dimension2d<irr::f32>(20.0f, 20.0f));
  laser->setID(id_flag_not_pickable);  // this ensures that we don't accidentally ray-pick it

  return laser;
}

irr::scene::IMeshSceneNode* add_level(irr::scene::ISceneManager* smgr)
{
  assert(smgr);

  // get mesh
  irr::scene::IAnimatedMesh* q3_level_mesh = smgr->getMesh("20kdm2.bsp");
  if (!q3_level_mesh) throw std::runtime_error("Cannot open mesh '20kdm2.bsp'");

  // add node resource
  irr::scene::IMeshSceneNode* q3_node =
    smgr->addOctreeSceneNode(q3_level_mesh->getMesh(0), nullptr, id_flag_is_pickable);
  if (!q3_node) throw std::runtime_error("Cannot add scene node");
  q3_node->setPosition(irr::core::vector3df(-1350, -130, -1400));

  // assign triangle selector
  irr::scene::ITriangleSelector* selector = smgr->createOctreeTriangleSelector(q3_node->getMesh(), q3_node, 128);
  if (!selector) throw std::runtime_error("Cannot create octree selector");
  q3_node->setTriangleSelector(selector);
  selector->drop();

  return q3_node;
}

void add_light(irr::scene::ISceneManager* smgr)
{
  assert(smgr);

  // add a light, so that the unselected nodes aren't completely dark.
  irr::scene::ILightSceneNode* light = smgr->addLightSceneNode(0, irr::core::vector3df(-60, 100, 400),
                                                               irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f), 600.0f);
  if (!light) throw std::runtime_error("Cannot add dynamic light scene node");

  light->setID(id_flag_not_pickable);  // make it an invalid target for selection.
}

}  // namespace

workshop::engine::engine(const std::string& irrlicht_media_path, irr::u32 width, irr::u32 height, irr::u32 bpp,
                         bool full_screen, bool stencil, bool vsync, device_type* type) :
    irrlicht_media_path_(irrlicht_media_path)
{
  if (type) {
    device_type_ = *type;
  } else {
    device_type_ = device_type::device_software;
  }

  try {
    event_receiver_ = new event_receiver;
    device_ =
      init_device(irrlicht_media_path_, width, height, bpp, full_screen, stencil, vsync, device_type_, event_receiver_);
    runtime_ = {device_->getVideoDriver(), device_->getSceneManager(), device_->getGUIEnvironment()};
    font_ = init_font(runtime_.guienv, irrlicht_media_path_);
    laser_ = init_laser(runtime_.smgr, runtime_.driver, irrlicht_media_path_);
    camera_ = new workshop::camera(runtime_.smgr, add_level(runtime_.smgr));
    add_light(runtime_.smgr);
  } catch (...) {
    delete camera_;
    if (device_) device_->drop();
    delete event_receiver_;
    throw;
  }
}

workshop::engine::~engine()
{
  delete camera_;
  if (device_) device_->drop();
  delete event_receiver_;
}

void workshop::engine::draw_label(const std::string& label)
{
  assert(font_);
  assert(runtime_.driver);

  font_->draw(
    std::wstring(label.begin(), label.end()).c_str(),
    irr::core::rect<irr::s32>(100, 10, static_cast<irr::s32>(runtime_.driver->getScreenSize().Width - 100), 60),
    irr::video::SColor(0xff, 0xff, 0xff, 0xf0), true, true);
}

void workshop::engine::process_collisions()
{
  assert(camera_->resource_);
  assert(runtime_.smgr);

  irr::core::line3d<irr::f32> ray;  // ray with finite length casted by the laser (attached to camera)
  ray.start = camera_->resource_->getPosition();
  ray.end = ray.start + (camera_->resource_->getTarget() - ray.start).normalize() * 1000.0f;

  irr::core::vector3df intersection;    // tracks the current intersection point with the level or a mesh
  irr::core::triangle3df hit_triangle;  // used to show which triangle has been hit
  irr::scene::ISceneCollisionManager* coll_man = runtime_.smgr->getSceneCollisionManager();
  irr::scene::ISceneNode* selected_scene_node =
    coll_man->getSceneNodeAndCollisionPointFromRay(ray, intersection, hit_triangle, id_flag_is_pickable);
  if (selected_scene_node) {
    // show laser and move it to position of detected collision with other node
    assert(laser_);
    laser_->setVisible(true);
    laser_->setPosition(intersection);

    // check if it is a collision with one of our characters and if yes cache it for further use
    if ((selected_scene_node->getID() & id_flag_is_highlightable) == id_flag_is_highlightable) {
      if (!selected_object_ || selected_object_->resource_ != selected_scene_node)
        selected_object_ = new object_handle(static_cast<irr::scene::IAnimatedMeshSceneNode*>(selected_scene_node));
    } else
      selected_object_ = nullptr;
  } else {
    selected_object_ = nullptr;

    // hide laser to simulate infinity distance
    laser_->setVisible(false);
  }
}

bool workshop::engine::run()
{
  assert(device_);
  assert(event_receiver_);

  return device_->run() && !event_receiver_->quit_;
}

bool workshop::engine::window_active()
{
  assert(device_);

  return device_->isWindowActive();
}

void workshop::engine::begin_scene()
{
  assert(runtime_.driver);
  assert(runtime_.smgr);
  assert(runtime_.guienv);
  assert(font_);

  if (!runtime_.driver->beginScene()) throw std::runtime_error("begin_scene() failed");

  runtime_.smgr->drawAll();
  runtime_.guienv->drawAll();
  const irr::s32 top = static_cast<irr::s32>(runtime_.driver->getScreenSize().Height - 50);
  const irr::s32 bottom = static_cast<irr::s32>(runtime_.driver->getScreenSize().Height);
  font_->draw(L"Press 'q' to exit", irr::core::rect<irr::s32>(10, top, 200, bottom),
              irr::video::SColor(0xff, 0xff, 0xff, 0xf0), false, true);
  process_collisions();
}

void workshop::engine::end_scene()
{
  assert(runtime_.driver);

  if (!runtime_.driver->endScene()) throw std::runtime_error("end_scene() failed");
}

void workshop::engine::yield()
{
  assert(device_);

  device_->yield();
}
