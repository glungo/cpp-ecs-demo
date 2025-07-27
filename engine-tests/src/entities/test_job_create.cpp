#include "include/job.h"
#include "include/component.h"
#include <cassert>
#include <iostream>
#include <memory>
using namespace entities;

namespace JobSystem
{
    namespace tests {

        DEFINE_COMPONENT(PositionComponent, 10)
            COMPONENT_MEMBER(float, x) = 0.0f;
            COMPONENT_MEMBER(float, y) = 0.0f;
            COMPONENT_MEMBER(float, z) = 0.0f;
        END_COMPONENT

        DEFINE_COMPONENT(VelocityComponent, 10)
            COMPONENT_MEMBER(float, vx) = 1.0f;
            COMPONENT_MEMBER(float, vy) = 1.0f;
            COMPONENT_MEMBER(float, vz) = 1.0f;
        END_COMPONENT

        class TestJob : public Job<PositionComponent, VelocityComponent> {
            public:
                TestJob() : Job<PositionComponent, VelocityComponent>("TestJob", [](float dt, JobCache<PositionComponent, VelocityComponent> cache) {
                    for (auto &tuple : cache) {
                        auto [pos, vel] = tuple;
                        pos->x += vel->vx * dt;
                        pos->y += vel->vy * dt;
                        pos->z += vel->vz * dt;
                    }
                }) {}
        };

        void test_job_create()
        {
            PositionComponent *pos = PositionComponent::Create();
            VelocityComponent *vel = VelocityComponent::Create();

            JobCache<PositionComponent, VelocityComponent> cache = {
                std::make_tuple(pos, vel)
            };

            TestJob job;
            job.SetCache(cache);
            job.Execute(1.0f);
            
            assert(pos->x == 1.0f && "Position x component should have been updated");
            assert(pos->y == 1.0f && "Position y component should have been updated");
            assert(pos->z == 1.0f && "Position z component should have been updated");

            PositionComponent::Destroy(pos);
            VelocityComponent::Destroy(vel);
        }

    } // namespace tests
} // namespace JobSystem

int main()
{
    JobSystem::tests::test_job_create();
    return 0;
}
