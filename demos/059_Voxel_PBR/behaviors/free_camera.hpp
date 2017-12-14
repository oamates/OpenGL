#include "../core/behavior.hpp"

struct FreeCamera : public Behavior
{
	void Update() override;
	FreeCamera() = default;
	~FreeCamera() = default;
};
