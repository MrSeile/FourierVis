#if 0

#include "UITools/UITools.h"

int main()
{
	ui::Vec2f v(1, 3);

	std::cout << v << " - " << v.Length() << " - " << v.Angle() << "\n";

	v.x = 3;
	v.y = 0;

	std::cout << v << " - " << v.Length() << " - " << v.Angle() << "\n";

	v.min = 0;
	v.max = -1.53f;

	std::cout << v << " - " << v.Length() << " - " << v.Angle() << "\n";
}

#endif