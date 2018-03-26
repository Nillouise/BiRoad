#pragma once
#include <string>

using std::string;

namespace Constant
{
	const string current_frame_numb = "current frame numb";
	const string press_key = "press key";
	const string self_id = "self id";
	const string item_delimiter = "|";
	const string equal_delimiter = "=";
	const string user_delimiter = "\n";
	//stater那个ini file用到的分割器
	const string starter_delimiter = "=";
	const string up = "Up";
	const string down = "Down";
	const string left = "Left";
	const string right = "Right";

	namespace GameMsg
	{
		const string width = "width";
		const string height = "height";
		const string pxWidth = "pxWidth";
		const string selfId = "selfId";
		const string randomSeed = "randomSeed";
		const string balls = "balls";
		const string snakes = "snakes";
		const string timeStamp = "timeStamp";
		const string press_key = Constant::press_key;
		const string isInitMsg = "init message";
	}
}
