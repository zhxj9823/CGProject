 #include "game_object.h"


GameObject::GameObject(): Position(0.0f, 0.0f, 0.0f), Velocity(0.0f,0.0f,0.0f), Destroyed(false) { }

GameObject::GameObject(glm::vec3 pos, glm::vec3 velocity): Position(pos), Velocity(velocity),Destroyed(false) { }

 void GameObject::Draw()
{
	 //renderer.DrawSprite(this->Sprite, this->Position, this->Size, this->Rotation, this->Color);
}