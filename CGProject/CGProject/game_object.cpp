 #include "game_object.h"


GameObject::GameObject(): Position(0.0f, 0.0f, 0.0f), Velocity(0.0f,0.0f,0.0f),  Destroyed(false) { }

GameObject::GameObject(glm::vec3 center) : Destroyed(false), Up(glm::vec3(0.0f, 1.0f, 0.0f))
{
	glm::vec3 size = glm::vec3(0.5f);
	double t = rand();
	double h = (rand() % 1000) / 101 - 5;
	glm::vec3 pos = glm::vec3(100 * cos(t), h, 100 * sin(t)) + center;
	t = rand();
	double v1 = (rand() % 1000) / 1001 + 10;
	glm::vec3 v = glm::vec3(v1*cos(t), 0, v1*sin(t));
	this->Position = pos;
	this->Size = size;
	this->Velocity = v;
	this->Right = glm::normalize(glm::cross(this->Velocity, this->Up));
}

GameObject::GameObject(glm::vec3 pos, glm::vec3 size, glm::vec3 velocity,glm::vec3 up) :
	Position(pos), Size(size), Velocity(velocity), Destroyed(false), Up(up),
	Acceleration(glm::vec3(0.0f,0.0f,0.0f)),WorldUp(glm::vec3(0.0f,1.0f,0.0f))
{
	this->Right = glm::normalize(glm::cross(this->Velocity, this->Up));
}

 void GameObject::Draw(Renderer & renderer,Camera & camera)
{
	 renderer.DrawPlane(this->Position, this->Size, this->Right , this->Up, camera);
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
 void GameObject::ProcessKeyboard(Camera_Movement direction, GLfloat deltaTime)
 {
	float t = 10.0f;
	if (direction == FORWARD)
		Acceleration = (WorldUp)* t*3.0f;
	if (direction == BACKWARD)
		Acceleration = -(WorldUp)* t*3.0f;
	if (direction == LEFT)
		Acceleration = -Right * t;
	if (direction == RIGHT)
		Acceleration = Right * t;
 }

 void GameObject::Update(GLfloat dt)
 {
	 this->Acceleration *= 0.5f;
	 this->Velocity += dt * this->Acceleration;
	 if (glm::dot(glm::normalize(Velocity), Velocity) > 2.0f)
		 Velocity = glm::normalize(Velocity)*2.0f;
	 if (glm::dot(glm::normalize(Velocity), Velocity) < 0.5f)
		 Velocity = glm::normalize(Velocity)*0.5f;
	 Velocity.y *= 0.5f;
	 this->Position += dt * this->Velocity;
	 this->Right = glm::normalize(glm::cross(this->Velocity, this->WorldUp));
	 this->Up = glm::normalize(glm::cross(this->Right,this->Velocity));
 }