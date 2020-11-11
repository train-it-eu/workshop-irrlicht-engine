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
#include <stdexcept>
#include <string>

namespace {

enum { id_flag_not_pickable = 0, id_flag_is_pickable = 1 << 0, id_flag_is_highlightable = 1 << 1 };

const std::wstring workshop_title = L"Modern C++ Design - Part I";

}  // namespace

/* ********************************* S E L E C T O R ********************************* */

workshop::selector::selector(engine& e, object_handle& object) :
    resource_(e.runtime().smgr.createTriangleSelector(object.resource_.get()))
{
}

/* ********************************* O B J E C T ********************************* */

namespace {

[[nodiscard]] irr::scene::IAnimatedMeshSceneNode& init_object_handle(workshop::engine::irr_runtime& r,
                                                                     workshop::object_handle::type type,
                                                                     const std::string& name,
                                                                     const std::filesystem::path& irrlicht_media_path)
{
  using ptr_type = gsl::not_null<nonstd::observer_ptr<irr::scene::IAnimatedMeshSceneNode>>;
  using enum workshop::object_handle::type;
  switch (type) {
    case faerie: {
      // add an MD2 node, which uses vertex-based animation
      auto path = irrlicht_media_path / "faerie.md2";
      nonstd::observer_ptr<irr::scene::IAnimatedMesh> mesh(r.smgr.getMesh(path.c_str()));
      if (!mesh) throw workshop::invalid_mesh_path("Cannot open mesh '" + path.string() + "'");
      ptr_type resource(r.smgr.addAnimatedMeshSceneNode(mesh.get(), 0, id_flag_is_pickable | id_flag_is_highlightable));
      resource->setScale(irr::core::vector3df(1.6f));
      resource->setMD2Animation(irr::scene::EMAT_POINT);
      resource->setAnimationSpeed(20.f);
      path = irrlicht_media_path / "faerie2.bmp";
      nonstd::observer_ptr<irr::video::ITexture> tex(r.driver.getTexture(path.c_str()));
      if (!tex) throw workshop::invalid_texture_path("Cannot open texture '" + path.string() + "'");
      irr::video::SMaterial material;
      material.setTexture(0, tex.get());
      material.Lighting = true;
      material.NormalizeNormals = true;
      resource->getMaterial(0) = material;
      resource->setName(name.c_str());
      return *resource;
    } break;

    case ninja: {
      // this B3D file uses skinned skeletal animation
      const auto path = irrlicht_media_path / "ninja.b3d";
      nonstd::observer_ptr<irr::scene::IAnimatedMesh> mesh(r.smgr.getMesh(path.c_str()));
      if (!mesh) throw workshop::invalid_mesh_path("Cannot open mesh '" + path.string() + "'");
      ptr_type resource(r.smgr.addAnimatedMeshSceneNode(mesh.get(), 0, id_flag_is_pickable | id_flag_is_highlightable));
      resource->setScale(irr::core::vector3df(10));
      resource->setAnimationSpeed(8.f);
      resource->getMaterial(0).NormalizeNormals = true;
      resource->getMaterial(0).Lighting = true;
      resource->setName(name.c_str());
      return *resource;
    } break;

    case dwarf: {
      // this X file uses skeletal animation, but without skinning
      const auto path = irrlicht_media_path / "dwarf.x";
      nonstd::observer_ptr<irr::scene::IAnimatedMesh> mesh(r.smgr.getMesh(path.c_str()));
      if (!mesh) throw workshop::invalid_mesh_path("Cannot open mesh '" + path.string() + "'");
      ptr_type resource(r.smgr.addAnimatedMeshSceneNode(mesh.get(), 0, id_flag_is_pickable | id_flag_is_highlightable));
      resource->setAnimationSpeed(20.f);
      resource->getMaterial(0).Lighting = true;
      resource->setName(name.c_str());
      return *resource;
    } break;

    case yodan: {
      // this mdl file uses skinned skeletal animation
      const auto path = irrlicht_media_path / "yodan.mdl";
      nonstd::observer_ptr<irr::scene::IAnimatedMesh> mesh(r.smgr.getMesh(path.c_str()));
      if (!mesh) throw workshop::invalid_mesh_path("Cannot open mesh '" + path.string() + "'");
      ptr_type resource(r.smgr.addAnimatedMeshSceneNode(mesh.get(), 0, id_flag_is_pickable | id_flag_is_highlightable));
      resource->setScale(irr::core::vector3df(0.8f));
      resource->getMaterial(0).Lighting = true;
      resource->setAnimationSpeed(20.f);
      resource->setName(name.c_str());
      return *resource;
    } break;
  }

  throw std::logic_error("never reaches here");
}

}  // namespace

workshop::object_handle::object_handle(engine& e, type t, const std::string& name) :
    resource_(&init_object_handle(e.runtime(), t, name, e.irrlicht_media_path()))
{
}

/* ********************************* C A M E R A ********************************* */

namespace {

irr::scene::ICameraSceneNode& init_camera(irr::scene::ISceneManager& smgr, irr::scene::IMeshSceneNode& level)
{
  nonstd::observer_ptr<irr::scene::ICameraSceneNode> resource(
    smgr.addCameraSceneNodeFPS(0, 50.0f, .3f, id_flag_not_pickable, 0, 0, true, 2.f));
  if (!resource) throw workshop::resource_creation_error("Cannot add camera scene node");

  workshop::droppable_res_ptr<irr::scene::ISceneNodeAnimator> anim(
    smgr.createCollisionResponseAnimator(level.getTriangleSelector(), resource.get(), irr::core::vector3df(30, 50, 30),
                                         irr::core::vector3df(0, -10, 0), irr::core::vector3df(0, 30, 0)));
  if (!anim)
    throw workshop::resource_creation_error(
      "Cannot create scene node animator for doing automatic collision detection and response");

  resource->addAnimator(anim.get());

  return *resource;
}

}  // namespace

workshop::camera::camera(irr::scene::ISceneManager& smgr, irr::scene::IMeshSceneNode& level) :
    resource_(init_camera(smgr, level))
{
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

[[nodiscard]] irr::video::E_DRIVER_TYPE convert(workshop::engine::device_type type)
{
  constexpr irr::video::E_DRIVER_TYPE irr_type[] = {irr::video::EDT_NULL, irr::video::EDT_SOFTWARE,
                                                    irr::video::EDT_DIRECT3D9, irr::video::EDT_OPENGL};
  constexpr size_t device_num = static_cast<size_t>(workshop::engine::device_type::last) + 1;
  static_assert(std::size(irr_type) == device_num, "Device translation table out of sync!");
  return irr_type[static_cast<int>(type)];
}

[[nodiscard]] gsl::not_null<workshop::droppable_res_ptr<irr::IrrlichtDevice>> init_device(
  const std::filesystem::path& irrlicht_media_path,
  const std::variant<workshop::window_params, workshop::full_screen_params>& screen_params,
  workshop::stencil_buffer stencil, workshop::vertical_sync vsync, workshop::engine::device_type device_type,
  workshop::engine::event_receiver& event_receiver)
{
  // create Irrlicht device - the most important object of the engine
  workshop::droppable_res_ptr<irr::IrrlichtDevice> device;

  if (std::holds_alternative<workshop::window_params>(screen_params)) {
    const auto& params = std::get<workshop::window_params>(screen_params);
    device.reset(irr::createDevice(convert(device_type), irr::core::dimension2d<irr::u32>(params.width, params.height),
                                   16,     // will be ignored
                                   false,  // windowed
                                   stencil, vsync, &event_receiver));
  } else {
    const auto& params = std::get<workshop::full_screen_params>(screen_params);
    device.reset(irr::createDevice(convert(device_type),
                                   irr::core::dimension2d<irr::u32>(params.window.width, params.window.height),
                                   static_cast<irr::u32>(params.bpp),
                                   true,  // full screen
                                   stencil, vsync, &event_receiver));
  }
  if (!device) throw workshop::resource_creation_error("Failed to create a device");

  // add Quake 3 map resources to Irrlicht local file system
  const auto path = irrlicht_media_path / "map-20kdm2.pk3";
  if (!device->getFileSystem()->addFileArchive(path.c_str()))
    throw workshop::invalid_archive_path("Cannot load archive '" + path.string() + "'");

  device->setWindowCaption(workshop_title.c_str());

  // get rid of the mouse cursor.  We'll use a billboard to show what we're looking at.
  device->getCursorControl()->setVisible(false);

  return device;
}

[[nodiscard]] irr::gui::IGUIFont& init_font(irr::gui::IGUIEnvironment& guienv,
                                            const std::filesystem::path& irrlicht_media_path)
{
  const auto path = irrlicht_media_path / "fonthaettenschweiler.bmp";
  nonstd::observer_ptr<irr::gui::IGUIFont> font(guienv.getFont(path.c_str()));
  if (!font) throw workshop::invalid_font_path("Cannot load font '" + path.string() + "'");
  return *font;
}

[[nodiscard]] irr::scene::IBillboardSceneNode& init_laser(irr::scene::ISceneManager& smgr,
                                                          irr::video::IVideoDriver& driver,
                                                          const std::filesystem::path& irrlicht_media_path)
{
  // add the laser
  nonstd::observer_ptr<irr::scene::IBillboardSceneNode> laser(smgr.addBillboardSceneNode());
  if (!laser) throw workshop::resource_creation_error("Cannot create a laser node");

  const auto path = irrlicht_media_path / "particle.bmp";
  nonstd::observer_ptr<irr::video::ITexture> laser_tex(driver.getTexture(path.c_str()));
  if (!laser_tex) throw workshop::invalid_texture_path("Cannot open texture '" + path.string() + "'");

  // init laser
  laser->setMaterialType(irr::video::EMT_TRANSPARENT_ADD_COLOR);
  laser->setMaterialTexture(0, laser_tex.get());
  laser->setMaterialFlag(irr::video::EMF_LIGHTING, false);
  laser->setMaterialFlag(irr::video::EMF_ZBUFFER, false);
  laser->setSize(irr::core::dimension2d<irr::f32>(20.0f, 20.0f));
  laser->setID(id_flag_not_pickable);  // this ensures that we don't accidentally ray-pick it

  return *laser;
}

[[nodiscard]] irr::scene::IMeshSceneNode& add_level(irr::scene::ISceneManager& smgr)
{
  // get mesh
  nonstd::observer_ptr<irr::scene::IAnimatedMesh> q3_level_mesh(smgr.getMesh("20kdm2.bsp"));
  if (!q3_level_mesh) throw workshop::invalid_mesh_path("Cannot open mesh '20kdm2.bsp'");

  // add node resource
  nonstd::observer_ptr<irr::scene::IMeshSceneNode> q3_node(
    smgr.addOctreeSceneNode(q3_level_mesh->getMesh(0), nullptr, id_flag_is_pickable));
  if (!q3_node) throw workshop::resource_creation_error("Cannot add scene node");
  q3_node->setPosition(irr::core::vector3df(-1350, -130, -1400));

  // assign triangle selector
  workshop::droppable_res_ptr<irr::scene::ITriangleSelector> selector(
    smgr.createOctreeTriangleSelector(q3_node->getMesh(), q3_node.get(), 128));
  if (!selector) throw workshop::resource_creation_error("Cannot create octree selector");
  q3_node->setTriangleSelector(selector.get());

  return *q3_node;
}

void add_light(irr::scene::ISceneManager& smgr)
{
  // add a light, so that the unselected nodes aren't completely dark.
  nonstd::observer_ptr<irr::scene::ILightSceneNode> light(smgr.addLightSceneNode(
    0, irr::core::vector3df(-60, 100, 400), irr::video::SColorf(1.0f, 1.0f, 1.0f, 1.0f), 600.0f));
  if (!light) throw workshop::resource_creation_error("Cannot add dynamic light scene node");

  light->setID(id_flag_not_pickable);  // make it an invalid target for selection.
}

}  // namespace

workshop::engine::engine(std::filesystem::path irrlicht_media_path,
                         const std::variant<window_params, full_screen_params>& screen_params,
                         stencil_buffer stencil /* = stencil_buffer(true) */,
                         vertical_sync vsync /* = vertical_sync(true) */,
                         device_type type /* = device_type::software */) :
    irrlicht_media_path_(std::move(irrlicht_media_path)),
    device_(init_device(irrlicht_media_path_, screen_params, stencil, vsync, type, event_receiver_)),
    runtime_{*device_->getVideoDriver(), *device_->getSceneManager(), *device_->getGUIEnvironment()},
    font_(init_font(runtime_.guienv, irrlicht_media_path_)),
    laser_(init_laser(runtime_.smgr, runtime_.driver, irrlicht_media_path_)),
    camera_(runtime_.smgr, add_level(runtime_.smgr))
{
  add_light(runtime_.smgr);
}

void workshop::engine::draw_label(const std::string& label)
{
  font_.draw(std::wstring(label.begin(), label.end()).c_str(),
             irr::core::rect<irr::s32>(100, 10, static_cast<irr::s32>(runtime_.driver.getScreenSize().Width - 100), 60),
             irr::video::SColor(0xff, 0xff, 0xff, 0xf0), true, true);
}

void workshop::engine::process_collisions()
{
  irr::core::line3d<irr::f32> ray;  // ray with finite length casted by the laser (attached to camera)
  ray.start = camera_.resource_.getPosition();
  ray.end = ray.start + (camera_.resource_.getTarget() - ray.start).normalize() * 1000.0f;

  irr::core::vector3df intersection;    // tracks the current intersection point with the level or a mesh
  irr::core::triangle3df hit_triangle;  // used to show which triangle has been hit
  irr::scene::ISceneCollisionManager& coll_man = *runtime_.smgr.getSceneCollisionManager();
  nonstd::observer_ptr<irr::scene::ISceneNode> selected_scene_node(
    coll_man.getSceneNodeAndCollisionPointFromRay(ray, intersection, hit_triangle, id_flag_is_pickable));
  if (selected_scene_node) {
    // show laser and move it to position of detected collision with other node
    laser_.setVisible(true);
    laser_.setPosition(intersection);

    // check if it is a collision with one of our characters and if yes cache it for further use
    if ((selected_scene_node->getID() & id_flag_is_highlightable) == id_flag_is_highlightable) {
      if (!selected_object_ || selected_object_->resource_.get() != selected_scene_node.get())
        selected_object_.emplace(*static_cast<irr::scene::IAnimatedMeshSceneNode*>(selected_scene_node.get()));
    } else
      selected_object_ = std::nullopt;
  } else {
    selected_object_ = std::nullopt;

    // hide laser to simulate infinity distance
    laser_.setVisible(false);
  }
}

void workshop::engine::begin_scene()
{
  if (!runtime_.driver.beginScene()) throw main_loop_error("begin_scene() failed");

  runtime_.smgr.drawAll();
  runtime_.guienv.drawAll();
  const irr::s32 top = static_cast<irr::s32>(runtime_.driver.getScreenSize().Height - 50);
  const irr::s32 bottom = static_cast<irr::s32>(runtime_.driver.getScreenSize().Height);
  font_.draw(L"Press 'q' to exit", irr::core::rect<irr::s32>(10, top, 200, bottom),
             irr::video::SColor(0xff, 0xff, 0xff, 0xf0), false, true);
  process_collisions();
}

void workshop::engine::end_scene()
{
  if (!runtime_.driver.endScene()) throw main_loop_error("end_scene() failed");
}
