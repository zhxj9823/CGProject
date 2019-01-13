 #include "game_object.h"


GameObject::GameObject(): Position(0.0f, 0.0f, 0.0f), Velocity(0.0f,0.0f,0.0f), Destroyed(false) { }

GameObject::GameObject(glm::vec3 pos, glm::vec3 size,glm::vec3 velocity):
	Position(pos),Size(size),Velocity(velocity),Destroyed(false),Up(glm::vec3(0.0f,1.0f,0.0f))
{
	this->Right = glm::normalize(glm::cross(Up, Velocity));
}

 void GameObject::Draw(Shader shader,Renderer & renderer,Camera & camera)
{
	 renderer.DrawPlane(shader, this->Position, this->Size, this->Up, this->Right, camera);
}
 glm::vec3 GameObject::Move(GLfloat dt)
 {
	 this->Position += dt * this->Velocity;
	 return this->Position;
 }
 double GameObject::Distance(glm::vec3 center)
 {
	 double d = glm::dot(Position - center,glm::normalize(Position - center));
	 return d;
 }