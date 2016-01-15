CollisionSystem.cpp
..\IsoARPG\src\ECS\CollisionSystem.cpp(18) : error C2512: 'CollisionSystem' : no appropriate default constructor available
..\IsoARPG\src\ECS\CollisionSystem.cpp(20) : error C2027: use of undefined type 'CollisionSystem'
        D:\C++Projects\VS\3DEnjon\IsoARPG\include\ECS/ComponentSystems.h(27) : see declaration of 'CollisionSystem'
..\IsoARPG\src\ECS\CollisionSystem.cpp(20) : error C2227: left of '->Manager' must point to class/struct/union/generic type
..\IsoARPG\src\ECS\CollisionSystem.cpp(30) : error C2027: use of undefined type 'CollisionSystem'
        D:\C++Projects\VS\3DEnjon\IsoARPG\include\ECS/ComponentSystems.h(27) : see declaration of 'CollisionSystem'
..\IsoARPG\src\ECS\CollisionSystem.cpp(30) : error C2227: left of '->Entities' must point to class/struct/union/generic type
..\IsoARPG\src\ECS\CollisionSystem.cpp(30) : error C2228: left of '.empty' must have class/struct/union
        D:\C++Projects\VS\3DEnjon\IsoARPG\include\ECS/ComponentSystems.h(27) : see declaration of 'CollisionSystem'
..\IsoARPG\src\ECS\CollisionSystem.cpp(30) : error C2228: left of '.size' must have class/struct/union
..\IsoARPG\src\ECS\CollisionSystem.cpp(35) : error C2027: use of undefined type 'CollisionSystem'
        D:\C++Projects\VS\3DEnjon\IsoARPG\include\ECS/ComponentSystems.h(27) : see declaration of 'CollisionSystem'
..\IsoARPG\src\ECS\CollisionSystem.cpp(35) : error C2227: left of '->Entities' must point to class/struct/union/generic type
..\IsoARPG\src\ECS\CollisionSystem.cpp(63) : error C3861: 'GetCollisionType': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(65) : error C3861: 'CollideWithEnemy': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(66) : error C3861: 'CollideWithProjectile': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(67) : error C3861: 'CollideWithItem': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(68) : error C3861: 'CollideWithEnemy': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(69) : error C3861: 'CollideWithEnemy': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(160) : error C3083: 'Random': the symbol to the left of a '::' must be a type
..\IsoARPG\src\ECS\CollisionSystem.cpp(160) : error C2039: 'Roll' : is not a member of 'Enjon'
..\IsoARPG\src\ECS\CollisionSystem.cpp(160) : error C3861: 'Roll': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(160) : error C2653: 'Collision' : is not a class or namespace name
..\IsoARPG\src\ECS\CollisionSystem.cpp(160) : error C3861: 'DropRandomLoot': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(264) : error C3083: 'Random': the symbol to the left of a '::' must be a type
..\IsoARPG\src\ECS\CollisionSystem.cpp(264) : error C2039: 'Roll' : is not a member of 'Enjon'
..\IsoARPG\src\ECS\CollisionSystem.cpp(264) : error C3861: 'Roll': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(274) : error C2653: 'Collision' : is not a class or namespace name
..\IsoARPG\src\ECS\CollisionSystem.cpp(274) : error C3861: 'DropRandomLoot': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(287) : error C3083: 'ResourceManager': the symbol to the left of a '::' must be a type
..\IsoARPG\src\ECS\CollisionSystem.cpp(287) : error C2039: 'GetTexture' : is not a member of 'Enjon::Input'
..\IsoARPG\src\ECS\CollisionSystem.cpp(287) : error C3861: 'GetTexture': identifier not found
..\IsoARPG\src\ECS\CollisionSystem.cpp(291) : error C3083: 'Random': the symbol to the left of a '::' must be a type
..\IsoARPG\src\ECS\CollisionSystem.cpp(291) : error C2039: 'Roll' : is not a member of 'Enjon'
..\IsoARPG\src\ECS\CollisionSystem.cpp(291) : error C2064: term does not evaluate to a function taking 2 arguments
..\IsoARPG\src\ECS\CollisionSystem.cpp(302) : error C3083: 'Random': the symbol to the left of a '::' must be a type
..\IsoARPG\src\ECS\CollisionSystem.cpp(302) : error C2039: 'Roll' : is not a member of 'Enjon'
..\IsoARPG\src\ECS\CollisionSystem.cpp(302) : error C2064: term does not evaluate to a function taking 2 arguments
..\IsoARPG\src\ECS\CollisionSystem.cpp(303) : error C3083: 'Random': the symbol to the left of a '::' must be a type
..\IsoARPG\src\ECS\CollisionSystem.cpp(303) : error C2039: 'Roll' : is not a member of 'Enjon'
..\IsoARPG\src\ECS\CollisionSystem.cpp(303) : error C2064: term does not evaluate to a function taking 2 arguments
..\IsoARPG\src\ECS\CollisionSystem.cpp(305) : error C2664: 'ECS::eid32 ECS::Systems::EntitySystem::CreateItem(ECS::Systems::EntityManager *,Enjon::Math::Vec3,Enjon::Math::Vec2,Enjon::Graphics::SpriteSheet *,ECS::Masks::EntityMask,ECS::Component::EntityType,char *,Enjon::Graphics::ColorRGBA8)' : cannot convert argument 2 from 'Enjon::Math::Vec2' to 'Enjon::Math::Vec3'
        No user-defined-conversion operator available that can perform this conversion, or the operator cannot be called
main.cpp
..\IsoARPG\src\main.cpp(240) : error C2653: 'Collision' : is not a class or namespace name
..\IsoARPG\src\main.cpp(240) : error C3861: 'Update': identifier not found
..\IsoARPG\src\main.cpp(245) : error C2027: use of undefined type 'CollisionSystem'
        D:\C++Projects\VS\3DEnjon\IsoARPG\include\ECS/ComponentSystems.h(27) : see declaration of 'CollisionSystem'
..\IsoARPG\src\main.cpp(245) : error C2227: left of '->Entities' must point to class/struct/union/generic type
..\IsoARPG\src\main.cpp(245) : error C2228: left of '.clear' must have class/struct/union
Generating Code...
Skipping... (no relevant changes detected)
Transform3DSystem.cpp
TestSystem.cpp
PlayerControllerSystem.cpp
LabelSystem.cpp
Entity.cpp
ComponentSystems.cpp
AttributeSystem.cpp
SpatialHash.cpp
Level.cpp
AnimationManager.cpp
Animation.cpp
Camera3D.cpp
Vec4.cpp
Vec3.cpp
Vec2.cpp
Mat4.cpp
Window.cpp
Timing.cpp
TextureCache.cpp
SpriteBatch.cpp
Sprite.cpp
ShaderManager.cpp
ResourceManager.cpp
picoPNG.cpp
IOManager.cpp
InputManager.cpp
ImageLoader.cpp
GLSLProgram.cpp
Game.cpp
Errors.cpp
Enjon.cpp
Camera2D.cpp
AABB.cpp
