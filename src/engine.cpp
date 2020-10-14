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

#include "engine.h"
#include <cassert>
#include <string>

namespace {

enum { id_flag_not_pickable = 0, id_flag_is_pickable = 1 << 0, id_flag_is_highlightable = 1 << 1 };

const std::wstring workshop_title = L"Modern C++ Design - Part I";

}  // namespace

/* ********************************* S E L E C T O R ********************************* */

int workshop::selector::init(engine* e, object_handle* object)
{
  assert(resource_ == nullptr);
  assert(e);
  assert(object);
  assert(object->resource_);

  workshop::engine::irr_runtime* r = e->runtime();
  assert(r->smgr);

  resource_ = r->smgr->createTriangleSelector(object->resource_);
  if (!resource_)
    return SELECTOR_INIT_FAIL;
  return SELECTOR_INIT_SUCCESS;
}

void workshop::selector::destroy()
{
  assert(resource_);

  resource_->drop();
  resource_ = nullptr;
}

workshop::selector::selector() : resource_(nullptr) {}

workshop::selector::~selector()
{
  if (resource_)
    destroy();
}

/* ********************************* O B J E C T ********************************* */

bool workshop::object_handle::resource_set(engine* e)
{
  assert(resource_ == nullptr);
  assert(e);

  workshop::engine::irr_runtime* r = e->runtime();
  assert(r->smgr);
  assert(r->driver);

  switch (type_) {
    case type_faerie: {
      // add an MD2 node, which uses vertex-based animation
      irr::scene::IAnimatedMesh* mesh = r->smgr->getMesh((e->irrlicht_path() + "/media/faerie.md2").c_str());
      if (!mesh)
        return false;
      resource_ = r->smgr->addAnimatedMeshSceneNode(mesh, 0, id_flag_is_pickable | id_flag_is_highlightable);
      resource_->setScale(irr::core::vector3df(1.6f));
      resource_->setMD2Animation(irr::scene::EMAT_POINT);
      resource_->setAnimationSpeed(20.f);
      irr::video::ITexture* tex = r->driver->getTexture((e->irrlicht_path() + "/media/faerie2.bmp").c_str());
      if (!tex) {
        resource_ = nullptr;
        return false;
      }
      irr::video::SMaterial material;
      material.setTexture(0, tex);
      material.Lighting = true;
      material.NormalizeNormals = true;
      resource_->getMaterial(0) = material;
      resource_->setName(name_->c_str());
    } break;

    case type_ninja: {
      // this B3D file uses skinned skeletal animation
      irr::scene::IAnimatedMesh* mesh = r->smgr->getMesh((e->irrlicht_path() + "/media/ninja.b3d").c_str());
      if (!mesh)
        return false;
      resource_ = r->smgr->addAnimatedMeshSceneNode(mesh, 0, id_flag_is_pickable | id_flag_is_highlightable);
      resource_->setScale(irr::core::vector3df(10));
      resource_->setAnimationSpeed(8.f);
      resource_->getMaterial(0).NormalizeNormals = true;
      resource_->getMaterial(0).Lighting = true;
      resource_->setName(name_->c_str());
    } break;

    case type_dwarf: {
      // this X file uses skeletal animation, but without skinning
      irr::scene::IAnimatedMesh* mesh = r->smgr->getMesh((e->irrlicht_path() + "/media/dwarf.x").c_str());
      if (!mesh)
        return false;
      resource_ = r->smgr->addAnimatedMeshSceneNode(mesh, 0, id_flag_is_pickable | id_flag_is_highlightable);
      resource_->setAnimationSpeed(20.f);
      resource_->getMaterial(0).Lighting = true;
      resource_->setName(name_->c_str());
    } break;

    case type_yodan: {
      // this mdl file uses skinned skeletal animation
      irr::scene::IAnimatedMesh* mesh = r->smgr->getMesh((e->irrlicht_path() + "/media/yodan.mdl").c_str());
      if (!mesh)
        return false;
      resource_ = r->smgr->addAnimatedMeshSceneNode(mesh, 0, id_flag_is_pickable | id_flag_is_highlightable);
      resource_->setScale(irr::core::vector3df(0.8f));
      resource_->getMaterial(0).Lighting = true;
      resource_->setAnimationSpeed(20.f);
      resource_->setName(name_->c_str());
    } break;

    case type_unknown:
      // nothing to do
      break;

    default:
      assert(0);
  }

  return true;
}

workshop::object_handle::object_handle(type t, const std::string* name) : type_(t), name_(name), resource_(nullptr) {}

bool workshop::object_handle::resource_set(irr::scene::IAnimatedMeshSceneNode* resource)
{
  assert(resource_ == nullptr);
  assert(resource);

  resource_ = resource;
  return true;
}

bool workshop::object_handle::operator==(const object_handle& rhs) const { return resource_ == rhs.resource_; }

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

int workshop::camera::init(irr::scene::ISceneManager* smgr, irr::scene::IMeshSceneNode* level)
{
  assert(resource_ == nullptr);
  assert(smgr);
  assert(level);

  resource_ = smgr->addCameraSceneNodeFPS(0, 50.0f, .3f, id_flag_not_pickable, 0, 0, true, 2.f);
  if (!resource_)
    return 1;

  irr::scene::ISceneNodeAnimator* anim =
      smgr->createCollisionResponseAnimator(level->getTriangleSelector(), resource_, irr::core::vector3df(30, 50, 30),
                                            irr::core::vector3df(0, -10, 0), irr::core::vector3df(0, 30, 0));
  if (!anim) {
    resource_ = nullptr;
    return 2;
  }

  resource_->addAnimator(anim);
  anim->drop();

  return 0;
}

workshop::camera::camera() : resource_(nullptr) {}

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
    if (event.KeyInput.PressedDown && event.KeyInput.Key == irr::KEY_KEY_Q)
      quit_ = true;
  return false;
}

/* ********************************* 3 D   E N G I N E ********************************* */

bool workshop::engine::internal_event_receiver_create()
{
  assert(event_receiver_ == nullptr);

  event_receiver_ = new (std::nothrow) event_receiver;
  return event_receiver_ != nullptr;
}

irr::video::E_DRIVER_TYPE workshop::engine::convert(device_type type)
{
  assert(type >= 0 && type < device_num);

  irr::video::E_DRIVER_TYPE irr_type[] = {irr::video::EDT_NULL, irr::video::EDT_SOFTWARE, irr::video::EDT_DIRECT3D9,
                                          irr::video::EDT_OPENGL};
  return irr_type[type];
}

int workshop::engine::init_device(irr::u32 width, irr::u32 height, irr::u32 bpp, bool full_screen, bool stencil, bool vsync)
{
  assert(device_ == nullptr);
  assert(runtime_.smgr == nullptr);
  assert(event_receiver_);

  // create Irrlicht device - the most important object of the engine
  device_ = irr::createDevice(convert(device_type_), irr::core::dimension2d<irr::u32>(width, height), bpp, full_screen,
                              stencil, vsync, event_receiver_);
  if (!device_)
    return 2;

  runtime_.smgr = device_->getSceneManager();

  // add Quake 3 map resources to Irrlicht local file system
  if (!device_->getFileSystem()->addFileArchive((irrlicht_path() + "/media/map-20kdm2.pk3").c_str())) {
    device_->drop();
    device_ = nullptr;
    return 3;
  }

  device_->setWindowCaption(workshop_title.c_str());

  // get rid of the mouse cursor.  We'll use a billboard to show what we're looking at.
  device_->getCursorControl()->setVisible(false);

  return 0;
}

bool workshop::engine::font()
{
  assert(font_ == nullptr);

  // load custom font
  if (!runtime_.guienv) {
    assert(device_);
    runtime_.guienv = device_->getGUIEnvironment();
  }
  font_ = runtime_.guienv->getFont((irrlicht_path_ + "/media/fonthaettenschweiler.bmp").c_str());
  return font_ != nullptr;
}

bool workshop::engine::add_laser()
{
  assert(laser_ == nullptr);

  // add the laser
  if (!runtime_.smgr) {
    assert(device_);
    runtime_.smgr = device_->getSceneManager();
  }

  laser_ = runtime_.smgr->addBillboardSceneNode();
  if (!laser_)
    return false;

  if (!runtime_.driver) {
    assert(device_);
    runtime_.driver = device_->getVideoDriver();
  }

  irr::video::ITexture* laser_tex = runtime_.driver->getTexture((irrlicht_path_ + "/media/particle.bmp").c_str());
  if (!laser_tex) {
    laser_ = nullptr;
    return false;
  }

  // init laser
  laser_->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
  laser_->setMaterialTexture(0, laser_tex);
  laser_->setMaterialFlag(irr::video::EMF_LIGHTING, false);
  laser_->setMaterialFlag(irr::video::EMF_ZBUFFER, false);
  laser_->setSize(irr::core::dimension2d<irr::f32>(20.0f, 20.0f));
  laser_->setID(id_flag_not_pickable);  // this ensures that we don't accidentally ray-pick it

  return true;
}

int workshop::engine::add_level(irr::scene::IMeshSceneNode** level)
{
  assert(level);
  assert(runtime_.smgr);

  // get mesh
  irr::scene::IAnimatedMesh* q3_level_mesh = runtime_.smgr->getMesh("20kdm2.bsp");
  if (!q3_level_mesh)
    return 1;

  // add node resource
  irr::scene::IMeshSceneNode* q3_node =
      runtime_.smgr->addOctreeSceneNode(q3_level_mesh->getMesh(0), nullptr, id_flag_is_pickable);
  if (!q3_node)
    return 2;
  q3_node->setPosition(irr::core::vector3df(-1350, -130, -1400));

  // assign triangle selector
  irr::scene::ITriangleSelector* selector =
      runtime_.smgr->createOctreeTriangleSelector(q3_node->getMesh(), q3_node, 128);
  if (!selector) {
    return 3;
  }
  q3_node->setTriangleSelector(selector);
  selector->drop();

  *level = q3_node;

  return 0;
}

int workshop::engine::create_camera(camera** c)
{
  if (camera_ == nullptr) {
    // create camera
    assert(c);

    camera_ = new (std::nothrow) camera;
    if (!camera_)
      return 1;

    if (!runtime_.smgr) {
      assert(device_);
      runtime_.smgr = device_->getSceneManager();
    }

    irr::scene::IMeshSceneNode* level = nullptr;
    if (add_level(&level)) {
      destroy_camera();
      return 2;
    }

    if (camera_->init(runtime_.smgr, level)) {
      destroy_camera();
      return 3;
    }
  }

  assert(camera_);
  *c = camera_;
  return 0;
}

void workshop::engine::destroy_camera()
{
  assert(camera_);

  delete camera_;
  camera_ = nullptr;
}

int workshop::engine::add_light()
{
  // add a light, so that the unselected nodes aren't completely dark.
  if (!runtime_.smgr) {
    assert(device_);
    runtime_.smgr = device_->getSceneManager();
  }
  irr::scene::ILightSceneNode* light = runtime_.smgr->addLightSceneNode(
      0, irr::core::vector3df(-60, 100, 400), irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f), 600.0f);
  if (!light)
    return 1;

  light->setID(id_flag_not_pickable);  // make it an invalid target for selection.
  return 0;
}

workshop::engine::engine(const std::string& irrlicht_path, device_type* type) :
    irrlicht_path_(irrlicht_path),
    device_type_(device_type::device_invalid),
    event_receiver_(nullptr),
    device_(nullptr),
    runtime_{nullptr, nullptr, nullptr},
    font_(nullptr),
    laser_(nullptr),
    camera_(nullptr),
    selected_object_(nullptr)
{
  if (type) {
    device_type_ = *type;
  } else {
    device_type_ = device_type::device_software;
  }
}

workshop::engine::~engine()
{
  if (device_)
    device_->drop();
  if (event_receiver_)
    delete event_receiver_;
}

void workshop::engine::draw_label(const std::string& label)
{
  assert(font_);
  assert(runtime_.driver);

  font_->draw(std::wstring(label.begin(), label.end()).c_str(),
              irr::core::rect<irr::s32>(100, 10, static_cast<irr::s32>(runtime_.driver->getScreenSize().Width - 100), 60),
              irr::video::SColor(0xff, 0xff, 0xff, 0xf0), true, true);
}

int workshop::engine::process_collisions()
{
  assert(camera_);
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
      if (!selected_object_ || selected_object_->resource_ != selected_scene_node) {
        selected_object_ = new (std::nothrow) object_handle(object_handle::type_unknown, nullptr);
        if (!selected_object_)
          return -1;
        selected_object_->resource_set(static_cast<irr::scene::IAnimatedMeshSceneNode*>(selected_scene_node));
      }
    } else
      selected_object_ = nullptr;
  } else {
    selected_object_ = nullptr;

    // hide laser to simulate infinity distance
    laser_->setVisible(false);
  }

  return 0;
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

bool workshop::engine::begin_scene()
{
  assert(runtime_.driver);
  assert(runtime_.smgr);
  assert(runtime_.guienv);
  assert(font_);

  if (!runtime_.driver->beginScene())
    return false;

  runtime_.smgr->drawAll();
  runtime_.guienv->drawAll();
  const irr::s32 top = static_cast<irr::s32>(runtime_.driver->getScreenSize().Height - 50);
  const irr::s32 bottom = static_cast<irr::s32>(runtime_.driver->getScreenSize().Height);
  font_->draw(L"Press 'q' to exit", irr::core::rect<irr::s32>(10, top, 200, bottom),
              irr::video::SColor(0xff, 0xff, 0xff, 0xf0), false, true);
  if (process_collisions() < 0)
    return false;

  return true;
}

bool workshop::engine::end_scene()
{
  assert(runtime_.driver);

  return runtime_.driver->endScene();
}

void workshop::engine::yield()
{
  assert(device_);

  device_->yield();
}
