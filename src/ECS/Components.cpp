#include <iostream>
#include "Components.h"
#include "../Lapis/LapisTypes.h"
#include "../Lapis/Engine.h"

#undef max
#define maximum(a, b) ((a) > (b) ? (a) : (b))
#define minimum(a, b) ((a) < (b) ? (a) : (b))
#define frictionConstant 0.02
#define gravityModifier 1

Vec3 pushDirection;
Vec3 friction;
float pushIntensity;



bool Rigidbody::IsLineIntersecting(const Vec3& start, const Vec3& end, std::shared_ptr<Transform> transformComponent)
{
    Vec3 minBounds = transformComponent->pos - Vec3(transformComponent->scale.x / 2.0f, transformComponent->scale.y / 2.0f, transformComponent->scale.z / 2.0f);
    Vec3 maxBounds = transformComponent->pos + Vec3(transformComponent->scale.x / 2.0f, transformComponent->scale.y / 2.0f, transformComponent->scale.z / 2.0f);
    float tMin = 0.0f;
    float tMax = std::numeric_limits<float>::infinity();
    for (int i = 0; i < 3; ++i) {
        float invDirection = 1.0f / (end[i] - start[i]);
        float tNear = (minBounds[i] - start[i]) * invDirection;
        float tFar = (maxBounds[i] - start[i]) * invDirection;
        if (invDirection < 0.0f) 
        {
            std::swap(tNear, tFar);
        }
        tMin = maximum(tNear, tMin);
        tMax = minimum(tFar, tMax);
        if (tMin > tMax) {
            return false;
        }
    }
    // The line segment intersects with the AABB
    return true;
}
std::shared_ptr<Entity> Rigidbody::Raycast(Transform raycastStart, float dist, std::vector<std::shared_ptr<Entity>>& liveObjects) {
    Vec3 raycastDirection = raycastStart.Forward();
    Vec3 raycastEnd = raycastStart.pos + raycastDirection * dist;
    for (auto& object : liveObjects) {
        auto transformComponent = object->GetComponent<Transform>();
        if (IsLineIntersecting(raycastStart.pos, raycastEnd, transformComponent)) {
            return object;
        }
    }
    return nullptr;
}

bool Rigidbody::BoxIntersect(BoxCollider a, BoxCollider b)
{
    return (
        a.minBounds.x <= b.maxBounds.x &&
        a.maxBounds.x >= b.minBounds.x &&
        a.minBounds.y <= b.maxBounds.y &&
        a.maxBounds.y >= b.minBounds.y &&
        a.minBounds.z <= b.maxBounds.z &&
        a.maxBounds.z >= b.minBounds.z
        );
}
void Rigidbody::SetColliderBounds(const Vec3& min, const Vec3& max)
{
    collider.minBounds = min;
    collider.maxBounds = max;
}


void Rigidbody::ProcessPhysics(std::vector<std::shared_ptr<Entity>>& liveObjects)
{
    // Find the player entity in the liveObjects vector
    std::shared_ptr<Entity> player = nullptr;
    for (const auto& entity : liveObjects) {
        if (entity->GetTag() == "player") {
            player = entity;
            break; // Stop searching once player is found
        }
    }

    if (!player) {
        // Player not found, handle error or return
        std::cerr << "Error: Player entity not found." << std::endl;
        return;
    }

    float movementSpeed = 100;
    Vec3 movementVec;
    if (GetAsyncKeyState(VK_LSHIFT))
    {
        movementSpeed *= 1.5;
    }

    // Calculate movement based on player input
    if (GetAsyncKeyState('A'))
        movementVec -= mainCamera.Right() * movementSpeed * deltaTime;
    if (GetAsyncKeyState('D'))
        movementVec += mainCamera.Right() * movementSpeed * deltaTime;
    if (GetAsyncKeyState('W'))
        movementVec += mainCamera.Forward() * movementSpeed * deltaTime;
    if (GetAsyncKeyState('S'))
        movementVec -= mainCamera.Forward() * movementSpeed * deltaTime;

    // Apply friction
    friction = player->GetComponent<Rigidbody>()->velocity * frictionConstant;
    player->GetComponent<Rigidbody>()->velocity -= friction;

    // Collision handling
    for (size_t i = 0; i < liveObjects.size(); ++i) {
        for (size_t j = i + 1; j < liveObjects.size(); ++j) {
            std::shared_ptr<Entity> entityA = liveObjects[i];
            std::shared_ptr<Entity> entityB = liveObjects[j];
            // Ensure that at least one of the entities is not the player entity
            if ((entityA != player|| entityB != player))
            {
                auto rigidbodyA = entityA->GetComponent<Rigidbody>();
                auto rigidbodyB = entityB->GetComponent<Rigidbody>();
                if (rigidbodyA && rigidbodyB) {
                    auto colliderA = rigidbodyA->collider;
                    auto colliderB = rigidbodyB->collider;
                    // Check for collision
                    if (BoxIntersect(rigidbodyA->collider, rigidbodyB->collider)) {

                        rigidbodyA->HandleCollision(entityB);
                        rigidbodyB->HandleCollision(entityA);
                    
                        // Identify the vertices involved in the collision
                        std::vector<Vec3> verticesA = {
                            colliderA.minBounds, // Front-bottom-left
                            Vec3(colliderA.minBounds.x, colliderA.minBounds.y, colliderA.maxBounds.z), // Front-bottom-right
                            Vec3(colliderA.minBounds.x, colliderA.maxBounds.y, colliderA.minBounds.z), // Front-top-left
                            Vec3(colliderA.minBounds.x, colliderA.maxBounds.y, colliderA.maxBounds.z), // Front-top-right
                            Vec3(colliderA.maxBounds.x, colliderA.minBounds.y, colliderA.minBounds.z), // Back-bottom-left
                            Vec3(colliderA.maxBounds.x, colliderA.minBounds.y, colliderA.maxBounds.z), // Back-bottom-right
                            Vec3(colliderA.maxBounds.x, colliderA.maxBounds.y, colliderA.minBounds.z), // Back-top-left
                            colliderA.maxBounds // Back-top-right
                        };
                        if (rigidbodyA->canCollide && rigidbodyB->canCollide)
                        {
                            // Compute distances from player to each vertex of entityA
                            float minDistance = std::numeric_limits<float>::max();
                            Vec3 closestVertex;
                            for (const Vec3& vertex : verticesA) {
                                float distance = Vec3::Distance(player->GetComponent<Transform>()->pos, vertex);
                                if (distance < minDistance) {
                                    minDistance = distance;
                                    closestVertex = vertex;
                                }
                            }

                            // Calculate direction from player to closest vertex
                            pushDirection = closestVertex - player->GetComponent<Transform>()->pos;
                            pushDirection.Normalize();
                            // Move the player away from the collider along the push direction
                            const float pushIntensity = sqrtf(player->GetComponent<Rigidbody>()->velocity.Magnitude()); // make it so it pushed back with equal force, netweon 2nd law or whatever
                            player->GetComponent<Rigidbody>()->velocity += pushDirection * pushIntensity;
                        }

                    }   
                }
            }
        }
    }
        
    //apply gravity
    if(!player->GetComponent<Rigidbody>()->grounded) player->GetComponent<Rigidbody>()->velocity = player->GetComponent<Rigidbody>()->velocity.y * gravityModifier * deltaTime;


    // Apply player input movement
    player->GetComponent<Rigidbody>()->velocity += movementVec;

    // Update player position based on velocity
    player->GetComponent<Transform>()->pos += player->GetComponent<Rigidbody>()->velocity * deltaTime;

   // std::cout << player->GetComponent<Rigidbody>()->velocity << " velocity|pos " << player->GetComponent<Transform>()->pos << std::endl;
}
