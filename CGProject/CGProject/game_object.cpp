 #include "game_object.h"


GameObject::GameObject(): Position(0.0f, 0.0f, 0.0f), Velocity(0.0f,0.0f,0.0f), Destroyed(false) { }

GameObject::GameObject(glm::vec3 pos, glm::vec3 size, Camera camera, glm::vec3 velocity):
	Position(pos),Size(size),camera(camera),Velocity(velocity),Destroyed(false) { }

 void GameObject::Draw(Renderer & renderer)
{
	 renderer.DrawPlane(this->Position, this->Size, this->camera);
}
 glm::vec3 GameObject::Move(GLfloat dt)
 {
	 this->Position += dt * this->Velocity;
	 return this->Position;
 }