#include "Bullet.h"
#include <iostream>



Bullet::Bullet(float yaw, float pitch, glm::vec4 bulletPos)
{
    this->bulletPos = bulletPos;

    this->yaw = yaw;
    this->pitch = pitch;
    glm::mat4 trans = glm::mat4(1.0f);
    // translatie punt waarond we gaan roteren naar origin == gunoffsett dus niet meer nodig 
    // doe rotatie
    trans = glm::rotate(trans, glm::radians(this->yaw + 90.0f), glm::vec3(0.0f, 1.0f, 0.0f)); // bij rotatie x op model = rotatie z op matrix
    trans = glm::rotate(trans, glm::radians(this->pitch), glm::vec3(0.0f, 0.0f, 1.0f));

    offsett = trans * glm::vec4(afstandPerLoop, 0.0f, 0, 1.0f);
}


void Bullet::updatePos()
{
    bulletPos.x = offsett.x + bulletPos.x;
    bulletPos.y = offsett.y + bulletPos.y;
    bulletPos.z = offsett.z + bulletPos.z;
}

void Bullet::draw()
{
    model = glm::mat4(1.0f);

    model = glm::translate(model, glm::vec3(bulletPos));
    model = glm::rotate(model, glm::radians(yaw), glm::vec3(0.0f, 1.0f, 0.0f));
    model = glm::rotate(model, glm::radians(pitch), glm::vec3(1.0f, 0.0f, 0.0f)); // model rotatie x op model, maar z in rotatiematrix (lang gezocht voor dit)
}
