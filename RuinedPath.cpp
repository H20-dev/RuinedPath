//-std=c++14 -Os -s
#include <bits/stdc++.h>
#include <windows.h>
#include <conio.h>
using namespace std;

// 全局变量

// 游戏状态变量
bool gameClear = false; // 基础剧情通关
bool advanced = false;  // 进阶剧情通关
bool zombieKing = false;// 尸王线
int girlRelat = 3;// 少女关系
int boyRelat = 3; // 少年关系
int death = 0;  // 死亡次数
int exitTry = 0;// 尝试退出次数
int loop = 1;   // 周目次数
bool speed = false;// 快进控制

const string SAVE_FILE = "save.ans";// 存档变量
const int VERSION = 2606;           // 版本号

// 资源变量
int bullet;// 子弹数量
int food;  // 食物数量

// 人物关系变量
bool girlLife = true;// 少女存活
bool haveBoy = false;// 少年是否入队
int boyWeapon = 0;   // 少年武器（0=无，1=枪，2=水管）

class Ends {
public:
	string name;
	bool unlocked = false;
	function<void()> func;
	Ends() {}
	Ends(string s2, function<void()> F) : name(s2), func(F) {}
	// 序列化（只存需要的数据）
	void serialize(ofstream& f) const {
		f.write((const char*)&unlocked, sizeof(unlocked));
	}
	// 反序列化（读回数据）
	void deserialize(ifstream& f) {
		f.read((char*)&unlocked, sizeof(unlocked));
	}
};
vector<Ends> badEnd;
vector<Ends> happyEnd;
vector<Ends> trueEnd;
vector<Ends> clue;


// 存档错误类型
enum class SaveErrorType {
	FILE_OPEN_FAILED,
	FILE_WRITE_FAILED,
	FILE_READ_FAILED,
	CHECKSUM_MISMATCH,
	VERSION_MISMATCH,
	INVALID_DATA
};
// 颜色
namespace color {
	const string basic = "\033[";

	const string red = basic + "31m";
	const string green = basic + "92m";
	const string yellow = basic + "33m";
	const string blue = basic + "34m";
	const string purple = basic + "35m";
	const string sky = basic + "96m";

	const string gray = basic + "90m";
	const string br_red = basic + "91m";

	const string bg_red = basic + "41m";
	const string bg_green = basic + "102m";
	const string bg_yellow = basic + "43m";
	const string bg_blue = basic + "44m";
	const string bg_gray = basic + "100m";

	const string rev = basic + "7m";
	const string bold = basic + "1m";
	const string reset = basic + "0m";
}
using namespace color;
// ===工具函数===
inline char getc() {
	return _getch();
}
// 随机数
inline int random(int min, int max) {
	static mt19937 gen(chrono::system_clock::now().time_since_epoch().count());
	uniform_int_distribution<int> dist(min, max);
	return dist(gen);
}
// 等待
inline void sleep(int ms) {
	Sleep(ms);
}
// 打印文字
inline void print(const string& text, bool enter = true) {
	for (char c : text) {
		if (gameClear && _kbhit()) {
			char key = _getch();
			if (key == 'F' || key == 'f') {
				speed = !speed;
			}
		}
		if (loop >= 2 && random(1, 30) <= loop) {
			sleep(min(120 * loop, 300) - speed * 80);
			cout << c;
		} else cout << c;
		if (c != ' ') sleep(30 - speed * 20);
	}
	cout << reset;
	if (enter) cout << endl;
	sleep(60 - speed * 40);
}
// 数字输入
inline int input(int mi, int ma) {
	char ch;
	int wrong = 0;
	ch = getc();
	while (ch < '0' || ch > '9') {
		print(red + "输入一个数字");
		wrong++;
		ch = getc();
	}
	int n =  ch - '0';

	while (n < mi || n > ma) {
		print(red + "输入正确的值");
		wrong++;
		ch = getc();
		while (ch < '0' || ch > '9') {
			print(red + "输入一个数字");
			wrong++;
			ch = getc();
		}
		n = ch - '0';
	}
	if (wrong >= 15) {
		print("“你还是人类吗？自动选择中……”");
		sleep(1200);
		return random(mi, ma);
	}
	return n;
}
// 清屏
inline void cls() {
	system("cls");
}
// 按键等待
inline void press() {
	print("按任意键继续...");
	getc();
	cls();
}
// 数字转中文大写
inline string numChinese(int num) {
	if (num < 0) return "";
	if (num == 0) return "零";

	const string digits[] = {"零", "壹", "贰", "叁", "肆", "伍", "陆", "柒", "捌", "玖"};
	const string units[] = {"仟", "佰", "拾", ""};
	const int weights[] = {1000, 100, 10, 1};

	string result;

	for (int i = 0; i < 4; i++) {
		int digit = num / weights[i]; // 取当前高位数字
		if (digit != 0) {
			// 拼接数字和对应单位
			result += digits[digit] + units[i];
		}
		num %= weights[i]; // 移除当前高位，处理下一位
	}
	// "壹拾"->"拾"
	if (result.size() >= 2 && result.substr(0, 4) == "壹拾") result = result.substr(2, 4);
	return result;
}
// 选项选择函数
inline int option(const string title, const vector<string>& options, bool isPause = true) {
	if (isPause) {
		print("按任意键继续...");
		getc();
	}
	int selected = 0;
	while (true) {
		cls();
		if (!title.empty()) cout << title << endl;

		cout << "选择：\n";
		for (int i = 0; i < options.size(); i++) {
			if (i == selected) {
				cout << br_red << "> " << options[i] << reset << endl;
			} else {
				cout << "  " << options[i] << endl;
			}
		}
		int ch;
		ch = _getch();
		if (ch == 0 || ch == 224) ch = _getch();
		if (ch == 72) selected = max(0, selected - 1); // 上
		else if (ch == 80) selected = min((int)options.size() - 1, selected + 1); // 下
		else if (ch == 13) return selected + 1; // 返回选项 Enter
		if (loop >= 2 && random(1, 5) == 1) {
			sleep(min(120 * loop, 300)); // 按键延迟
		}
		if (ch >= '1' && ch <= '9') { // 数字输入
			int num = ch - '0';
			if (num <= options.size()) {
				return num;
			}
		}
		if (ch == 'H' || ch == 'h') {
			char key = _getch();
			if (key == '2') {
				key = _getch();
				if (key == '0') {
					print(purple + rev + "“翻源代码？太可恨了！” ——The Creator H20");
					print(sky + "===== THE END - CHEATING =====");
					press();
					exit(0);
				}
			}
		}
	}
}

// 显示当前状态（子弹+食物）
inline void showStatus() {
	cls();
	print("【当前状态】");
	string bulletBar;
	if (bullet <= 9)bulletBar += " ";
	bulletBar += bg_red;
	for (int i = 0; i < 15; i++) {
		if (i < bullet)bulletBar += " ";
		else if (i == bullet)bulletBar += reset;
	}

	string foodBar;
	if (food <= 9)foodBar += " ";
	foodBar += bg_yellow;
	for (int i = 0; i < 15; i++) {
		if (i < food)foodBar += " ";
		else if (i == food)foodBar += reset + " ";
	}

	print(string("子弹：") + "[" + to_string(bullet) + "] " + bulletBar + reset);
	print(string("食物：") + "[" + to_string(food) + "] " + foodBar + reset);

	string icons = "";
	if (gameClear) icons += "通 "; //通关
	if (zombieKing) icons += "尸 "; //尸王线
	if (advanced) icons += "进 ";  //进阶剧情
	if (loop >= 2) icons += "超 "; //周目
	if (!icons.empty()) print("特殊状态：" + icons);
}
void resetGameState() {
	bullet = 0;
	food = 0;
	girlLife = true;
	haveBoy = false;
	boyWeapon = 0;
}
void resetGameLoop() {
	print(blue + "下一周目：开始。");
	loop ++;
	gameClear = true;
	advanced = false;
	death /= 2;
	girlRelat = 3 + girlRelat * 0.5;
	boyRelat = 3 + boyRelat * 0.5;
	zombieKing = false;
}

// 统计结局数组
int countUnlocked(const vector<Ends>& arr) {
	int intg = 0;
	for (Ends i : arr) {
		if (i.unlocked) intg++;
	}
	return intg;
}
// 结局显示
void showEnd(vector<Ends> arr, string clr, string title) {
	print("\n===== " + title + " =====  (" + to_string(countUnlocked(arr)) + "/" + to_string(arr.size() - 1) + ")");
	for (int i = 1; i < arr.size(); i++) {
		if (arr[i].unlocked) {
			print( clr + "[" + numChinese(i) + "] 已解锁 " + arr[i].name );
		} else {
			print( gray + "[" + numChinese(i) + "] 未解锁 " + "？？？？");
		}
	}
}
//进度
void showAdv() {
	print("\n===== 阶级 =====");
	if (gameClear) print("基础剧情 · 破局之始");
	if (advanced) print("进阶剧情 · 真貌初显");
	if (zombieKing) print("尸王线 · 生而彷徨");
	if (loop >= 2) print("新周目 · 重始新篇");
	print("死亡次数：" + numChinese(death));
}

// 存档错误信息
void showSaveError(SaveErrorType error, const string& operation) {
	string errorMsg;
	switch (error) {
		case SaveErrorType::FILE_OPEN_FAILED:
			errorMsg = "无法打开存档文件";
			break;
		case SaveErrorType::FILE_WRITE_FAILED:
			errorMsg = "存档写入失败";
			break;
		case SaveErrorType::FILE_READ_FAILED:
			errorMsg = "存档读取失败";
			break;
		case SaveErrorType::CHECKSUM_MISMATCH:
			errorMsg = "存档文件损坏";
			break;
		case SaveErrorType::VERSION_MISMATCH:
			errorMsg = "存档版本不兼容";
			break;
		case SaveErrorType::INVALID_DATA:
			errorMsg = "存档数据无效";
			break;
		default:
			errorMsg = "未知错误";
	}
	print(red + "[" + operation + "失败]" + errorMsg);
}

bool checkSaveExists() {
	try {
		ifstream file(SAVE_FILE); 	  // 尝试以读模式打开文件
		bool exists = file.good();    // 文件存在且可打开则为true
		file.close();                 // 立即关闭文件
		return exists;
	} catch (...) {
		print(yellow + "检查存档时发生错误");
		return false;
	}
}

// 保存游戏
bool saveGame() {
	ofstream file(SAVE_FILE, ios::binary);
	if (!file) {
		showSaveError(SaveErrorType::FILE_OPEN_FAILED, "保存");
		return false;
	}

	// 写版本
	file.write((char*)&VERSION, sizeof(VERSION));

	// 写所有游戏数据
#define W(x) file.write((char*)&x, sizeof(x))
	W(gameClear);
	W(advanced);
	W(death);
	W(exitTry);
	W(loop);
	W(girlRelat);
	W(boyRelat);
	W(zombieKing);
#undef W

	// 只写结局解锁状态（安全）
	for (auto& e : badEnd)  e.serialize(file);
	for (auto& e : happyEnd)e.serialize(file);
	for (auto& e : trueEnd) e.serialize(file);
	for (auto& e : clue)    e.serialize(file);

	// 简单校验
	uint32_t checksum = 0x3241;
	file.write((char*)&checksum, 4);
	file.close();
	return true;
}

// 加载游戏
bool loadGame() {
	if (!checkSaveExists()) return false;

	ifstream file(SAVE_FILE, ios::binary);
	if (!file) {
		showSaveError(SaveErrorType::FILE_OPEN_FAILED, "加载");
		return false;
	}

	// 读版本
	int ver;
	file.read((char*)&ver, sizeof(ver));
	if (ver != VERSION) {
		file.close();
		showSaveError(SaveErrorType::VERSION_MISMATCH, "加载");
		return false;
	}

	// 读所有数据
#define R(x) file.read((char*)&x, sizeof(x))
	R(gameClear);
	R(advanced);
	R(death);
	R(exitTry);
	R(loop);
	R(girlRelat);
	R(boyRelat);
	R(zombieKing);
#undef R

	// 读解锁状态
	for (auto& e : badEnd)  e.deserialize(file);
	for (auto& e : happyEnd)e.deserialize(file);
	for (auto& e : trueEnd) e.deserialize(file);
	for (auto& e : clue)    e.deserialize(file);

	// 读校验，不影响
	uint32_t cs;
	file.read((char*)&cs, 4);
	file.close();
	return true;
}

// 线索集成
void cluei(int n) {
	print(sky + "【线索·星光】第" + numChinese(n) + "章");
	print(sky + clue[n].name);
	clue[n].unlocked = true;
}
//食物集成判断
bool foodi(int num) {
	if (num <= 0) {
		food += num;
		return false;
	}
	print("按任意键食用...");
	getc();
	food -= num;
	if (food < 0)return true;
	return false;
}
//子弹集成判断
bool bulleti(int num) {
	if (num <= 0) {
		bullet += num;
		return false;
	}
	for (int i = 1; i <= num; i++) {
		bullet--;
		if (bullet <= 0)return true;
		print("按任意键开枪...");
		getc();

		int hitRate = 70; // 基础命中率
		hitRate += min(20, int(death * 0.4)); // 死亡提升
		if (zombieKing) hitRate = 100; // 尸王必中
		if (loop >= 2) hitRate += 10 * loop; // 熟练度提升

		if (random(1, 100) <= hitRate) {
			print("命中！丧尸倒地");
		} else {
			print("打偏！丧尸还活着");
			i--;
			if (bullet <= 0)return true;
		}
		sleep(100);
	}
	return false;
}
// 伪装bug
void fakeBug(int bugType = random(1, 40 - loop * 3)) {
	if (loop < 2) return;
	switch (bugType) {
		case 1: // 计算器
			for (int i = 1; i <= 6; i++) system("start calc");
			break;
		case 2:
			print(purple + "===== 内存溢出警告 =====");
			sleep(600 * loop);
			cls();
			break;
		case 3: // 假的程序崩溃提示
			print(red + bold + "程序异常 0xC000041D: 主线程退出");
			sleep(1000 * loop);
			cls();
			break;
		case 4: // 文字重复输出
			print(red + "你你你你选选选选择择择择错错错错误误误误", false);
			sleep(600 * loop);
			cout << reset << "\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b"; // 回退删除乱码
			break;
		case 6: // 假的存档损坏提示
			print(yellow + "[警告] 存档文件 CRC 校验失败 (0xC000" + to_string(random(10000, 99999)) + ")");
			sleep(1200 * loop);
			cls();
			break;
		case 7: // 界面元素错位
			cout << string(10, '\n');
			cout << string(25, ' ');
			print(sky + "<<<<<<<<< 渲染层偏移 >>>>>>>>>");
			sleep(800 * loop);
			cls();
			break;
		case 8: // 按键无响应假象（仅延迟）
			print("按任意键继续...", false);
			sleep(2000 * loop);
			break;
		case 9: //cmd
			for (int i = 1; i <= 10; i++) system("start cmd");
			break;
		case 10:
			for (int i = 1; i <= 25; i++) system("start powershell");
			break;
		default:
			break;
	}
}
void initClue() {
	const vector<string> clName = {
		"0-诡枪异食", "1-迟变少年", "2-实验无尽", "3-域中异象",
		"4-早变异女", "5-实验双体", "6-破局而退", "7-死亡迷局", "8-复活循环",
		"9-变异溯源", "10-记忆之键", "11-域外之界", "12-虚实之辨", "13-自主觉醒",
		"14-同化终局", "15-双体共鸣", "16-终得团聚", "17-性情相容", "18-投机不巧",
		"19-无尽创世", "20-稳态调节", "21-域界平衡", "22-记忆共生", "23-轻于鸿毛"
		"24-新生残途"
	};
	for (int i = 0; i < clName.size(); i++) clue.push_back(Ends(clName[i], []() {}));
}
// 结局函数

void initBadEnd() {
	badEnd.resize(31);
	auto BE = [](int n) {
		print(red + "【终局·憾恨】第" + numChinese(n) + "幕");
		print(red + badEnd[n].name);
		badEnd[n].unlocked = 1;
		death++;
	};
	badEnd[1] = Ends("1-饥馑殒命", [&BE]() {
		press();
		print("粮囊已空，你静坐于破败的角落，身躯因饥饿日渐消瘦。");
		print("指尖划过干裂的唇，连呼喊的力气也已耗尽，在寂静中咽下了最后一口气。");
		BE(1);
		press();
	});
	badEnd[2] = Ends("2-破密无门", [&BE]() {
		press();
		print("你皱着眉反复尝试，指尖因焦躁微微颤抖，按钮上的数字冰冷却熟悉。");
		print("请输入密码：");
		print("123456");
		sleep(100);
		print(red + "密码错误！警报声骤然响起");
		print("000000");
		sleep(100);
		print(red + "密码错误！防御系统启动");
		print("666666");
		sleep(100);
		print(red + bold + "密码错误！目标已锁定——入侵者");
		print("");
		print("一道激光破空而来，你甚至来不及反应，便已倒在血泊之中");
		BE(2);
		press();
	});
	badEnd[3] = Ends("3-神智崩摧", [&BE]() {
		press();
		print("长久的等待与绝望如同潮水，终于漫过了理智的堤坝，你陷入了彻底的癫狂。");
		BE(3);
		if (happyEnd[1].unlocked) cluei(3);
		press();
	});
	badEnd[4] = Ends("4-独往基覆", [&BE]() {
		press();
		print("当你完成任务归来时，眼前的景象令你如坠冰窟——");
		print("幸存者基地已被丧尸攻破，残垣断壁间，只余丧尸的嘶吼与冰冷的死寂。");
		print("曾并肩作战的同伴，皆已化作了亡魂，无一人幸免。");
		BE(4);
		press();
	});
	badEnd[5] = Ends("5-基地沦亡", [&BE]() {
		press();
		print("丧尸的浪潮如同黑色的洪水，汹涌地冲击着基地的防线。");
		print("尽管基地的众人皆拼尽全力抵抗，血肉之躯终究难敌滔天尸潮。");
		print("防线崩裂的刹那，丧尸涌入，你被淹没在狰狞的爪牙之中，最终殒命于此。");
		BE(5);
		press();
	});
	badEnd[6] = Ends("6-变异失智", [&BE]() {
		press();
		print("一股异样的燥热从四肢百骸涌起，你能清晰地感觉到，身体正在被病毒吞噬。");
		print("意识如同风中残烛，渐渐模糊，腹中升腾起对人肉的疯狂渴望。");
		print("最终，你眼前一黑，彻底失去了作为人的一切，沦为了行尸走肉。");
		BE(6);
		press();
	});
	badEnd[7] = Ends("7-作者裁决", [&BE]() {
		press();
		print("冥冥之中，一股不可抗拒的力量降临，你尚未察觉，便已魂归黄泉——");
		print("你被这世界的缔造者，亲手抹杀。");
		BE(7);
		cluei(18);
		press();
	});
	badEnd[8] = Ends("8-异女噬身", [&BE]() {
		press();
		print("椅上坐着的，是个十五岁上下的少年，眉眼间带着与年龄不符的冷漠。");
		print("你正为这突兀的景象惊愕，少年却缓缓站起，望向你身后的少女。");
		print("“差不多了吧。”少女的声音响起，带着一丝诡异的笑意。");
		print("“可以了。”少年的回答简洁而冰冷。");
		print("下一瞬，脖颈处传来刺骨的剧痛，鲜血喷涌而出。");
		print("你失去意识前的最后一幕，是少女俯身，咬向你脖颈的模样。");
		if (gameClear)	print("“这不是应该有的结局……”");
		BE(8);
		cluei(4);
		press();
	});
	badEnd[9] = Ends("9-阅记遭噬", [&BE]() {
		press();
		print("正当你沉浸在笔记的内容中时，脖颈处骤然传来剧痛——被人狠狠咬住。");
		print("临死前，你隐约听到少年的声音，轻得如同叹息：“再来一次吧。”");
		BE(9);
		cluei(5);
		press();
	});
	badEnd[10] = Ends("10-麻木度世", [&BE]() {
		press();
		print("你在荒野中，寻到了另一处幸存者基地，弹尽粮绝的你，选择加入其中。");
		print("此后的日子，单调得如同复制粘贴：白日外出搜寻物资，夜晚归巢蜷缩度日。");
		print("“本该是这样的吧？”你时常这样问自己，却只得到满心的空洞。");
		print("日复一日的孤独，终于磨去了你活下去的所有意义。");
		print("你选择了结束自己的生命，在末世的寂静中，了却残生。");
		BE(10);
		press();
	});
	badEnd[11] = Ends("11-轻敌丧身", [&BE]() {
		press();
		print("你仓促间朝丧尸扣动扳机，却听见枪膛发出一声空响——子弹尚未上膛。");
		print("丧尸的利爪转瞬即至，你被一口咬断喉咙，倒在了血泊之中。");
		BE(11);
		press();
	});
	badEnd[12] = Ends("12-称王失意", [&BE]() {
		press();
		print("作为至高无上的王，你看向站在你下面的所有人...");
		print("歼灭一切有生力量，不留一个人头！");
		print("你的旨令得到了落实，三日之内，方圆百里内已无一人");
		print("上万人的军团连连几天都找不到所需的食物，世界的远方寂寞如死水");
		print("为了食物，大家互相啃咬着、吞食着，应该没有人能躲过这场灾难");
		print("你不幸在睡梦中，被部下杀死……");
		print("“果然，最可恨的不是反派，而是生存的本能”");
		BE(12);
		press();
	});
	badEnd[13] = Ends("13-崩解猎援", [&BE]() {
		press();
		print("长久的等待与绝望如同潮水，终于漫过了理智的堤坝，你陷入了彻底的癫狂。");
		if (zombieKing) {
			print("当政府救援队推开你家的门时，你嘶吼着扑向那抹鲜活的人气。");
			print("他们试图将你击毙，却被你先发制人，一枪击倒在前。");
			print("你如同狩猎的凶兽，将救援队逐一屠戮，鲜血染红了原本安宁的居所。");
			print("当最后一人倒下时，你俯身，开始享用这场血腥的“盛宴”。");
			BE(13);
			cluei(9);
			press();
			return;
		}
		if (happyEnd[1].unlocked) cluei(3);
		press();
	});
	badEnd[14] = Ends("14-出界无讯", [&BE]() {
		press();
		print("一股强烈的撕扯感骤然袭来，仿佛有无形的手，要将你的灵魂从躯体中剥离。");
		print("这般剧痛，远非血肉之躯所能承受，你在极致的痛苦中，失去了生命。");
		BE(14);
		cluei(11);
		press();
	});
	badEnd[15] = Ends("15-熟迹反灭", [&BE]() {
		press();
		print("你忽然察觉，笔记上的字迹竟带着几分熟悉，仿佛是刚被人写下不久。");
		print("正当你凝神思索时，脖颈处骤然传来剧痛——被人狠狠咬住。");
		print("临死前，你隐约听到少年的声音，轻得如同叹息：“再来一次吧。”");
		BE(15);
		cluei(12);
		press();
	});
	badEnd[16] = Ends("16-星光尽灭", [&BE]() {
		press();
		print("你底 干什么？世如此");
		print("为到么什这在个界  卡顿？");
		for (int i = 1; i <= 5; i++) fakeBug();
		print("每  次检测错目周增 时加，线索内存 都会加增，的用 占会致导卡顿，甚    至B u     g");
		print("系统  触发误，紧急栈修复中...过高 ");
		BE(16);
		press();
	});
	badEnd[17] = Ends("17-杀女被败", [&BE]() {
		print("你的杀意被她察觉，她先发制人，一颗子弹精准地穿透了你的心脏。");
		BE(17);
		press();
	});
	badEnd[18] = Ends("18-弹尽群噬", [&BE]() {
		press();
		print("三只丧尸嘶吼着蜂拥而上，你被瞬间扑倒在地，根本来不及挣扎。");
		print("尖锐的牙齿狠狠刺穿你的喉咙，鲜血如泉涌，意识如潮水般迅速褪去……");
		if (gameClear) {
			print("意识消散的最后一刻，你脑海中闪现无数丧尸扑来的画面……");
			print("“又是这样吗？”");
		}
		BE(18);
		press();
	});
	badEnd[19] = Ends("19-孤存终亡", [&BE]() {
		press();
		print("你携着无尽的食物，独自流浪在末世的荒原之上，却再也未曾遇见任何活物。");
		print("孤独如同藤蔓，缠绕着你的心脏，日复一日，终至窒息。");
		print("你终于选择了结束自己的生命，在空无一人的世界里，归于沉寂。");
		BE(19);
		press();
	});
	badEnd[20] = Ends("20-精英丧噬", [&BE]() {
		press();
		print("精英丧尸的速度远超你的想象，它灵巧地躲过你的子弹，瞬间便扑至你面前。");
		print("锋利的爪子撕开了你的胸膛，鲜血染红了脚下的土地，你轰然倒地……");
		if (gameClear) {
			print("意识消散的最后一刻，你脑海中闪现两只精英丧尸扑来的画面……");
			print("“又是这样吗？”");
		}
		BE(20);
		press();
	});
	badEnd[21] = Ends("21-途竭而终", [&BE]() {
		press();
		print("你与少年结伴，踏上了寻找安全区的路途，然末世的危险，远非你们所能预料。");
		print("一次搜寻物资时，少年不慎触发丧尸陷阱，你为救他，耗尽了所有子弹。");
		print("最终，你们被丧尸团团包围，力竭而亡，尸骨湮灭在尸潮之中。");
		BE(21);
		press();
	});
	badEnd[22] = Ends("22-少叛遭祸", [&BE]() {
		press();
		print("深夜露营时，少年趁你熟睡，偷走了所有食物与剩余的子弹，消失在夜色中。");
		print("你醒来时，已是一无所有，饥饿与绝望将你吞噬，最终倒在荒野之中。");
		print("丧尸的嘶吼由远及近，成为了你最后的丧钟。");
		BE(22);
		press();
	});
	badEnd[23] = Ends("23-安离反目", [&BE]() {
		press();
		print("你与少年侥幸逃离商场，然后续的旅程中，食物日渐耗尽。");
		print("为了最后一份食物，你们反目成仇，少年用你赠予的武器，将你击伤在地。");
		print("你躺在冰冷的地面，看着他拿着食物仓皇逃走，最终被赶来的丧尸吞噬。");
		BE(23);
		press();
	});
	badEnd[24] = Ends("24-杀少成魔", [&BE]() {
		press();
		print("你闭上眼睛，狠下心扣动扳机，少年应声倒地，鲜血染红了他的衣衫。");
		print("杀死同伴的罪恶感如影随形，最终令你精神崩溃，变得嗜血好杀。");
		print("你成了末世中人人喊打的恶魔，最终被其余幸存者联合围剿，毙于乱枪之下。");
		BE(24);
		press();
	});
	badEnd[25] = Ends("25-携少饥终", [&BE]() {
		press();
		print("你携着被感染的少年，在末世中艰难前行，然食物很快便消耗殆尽。");
		print("你们相拥着蜷缩在废弃的车厢里，在饥饿与寒冷中，一同走向了死亡。");
		BE(25);
		press();
	});
	badEnd[26] = Ends("26-异少噬身", [&BE]() {
		press();
		print("食物耗尽的刹那，少年彻底变异，理智被病毒吞噬，化作了狰狞的丧尸。");
		print("他扑向你，锋利的牙齿咬断了你的颈动脉，鲜血溅满了他的脸庞。");
		print("你到死都不敢相信，自己拼尽全力守护的人，最终竟成了终结你的侩子手。");
		BE(26);
		press();
	});
	badEnd[27] = Ends("27-首领灭知", [&BE]() {
		print("他看向你，目光冷冽，稍加思索后，缓缓开口：");
		print(red + bold + "你知道的太多了，已无法重置，只能被销毁！");
		print("你的意识最终消散在乱码之中");
		BE(27);
		press();
	});
	badEnd[28] = Ends("28-空城孤王", [&BE]() {
		press();
		if (loop >= 2) {
			print("你站在城市顶端，俯瞰着俯首帖耳的尸群，却连一丝胜利的喜悦都无——记忆告诉你，这空城本就是定制的牢笼。");
			print("这些丧尸是管理员三号的实验造物，这死寂的风鸣，是虚拟场景的背景音，连城市的轮廓，都与上一次分毫不差。");
			print("“我成了尸王，却仍困在管理员的手心里。”你低声自语，指尖划过冰冷的建筑，怀念起人类的温度、语言的喧嚣。");
		} else {
			print("你站在城市顶端，俯瞰着被尸群掌控的空城，却感受不到丝毫喜悦");
			print("尸群没有意识，无法交流，世界只剩下死寂的风鸣");
			print("你开始怀念人类的温度、语言的喧嚣，甚至是末世的挣扎...");
		}
		print("");
		print(purple + "你成了空城的唯一王者，知道一切，却无人可统治，无人可倾诉。");
		print(purple + "当最后一丝人类意识消散时，你终于明白 —— 尸王的王座，本就是孤独的坟墓。");
		BE(28);
		press();
	});
	badEnd[29] = Ends("29-循环囚笼", [&BE]() {
		press();
		if (loop >= 3) {
			print("你站在实验室中央，手环上的周目数字跳至“叁”，却发现所有出口都已锁死。");
			print("总管理员的电子音响起：“三周目触发终极囚笼，实验体3号将永久循环此场景”");
			print("你尝试了所有方法——破译密码、摧毁控制台、甚至同化丧尸，却始终无法突破这层无形的壁垒。");
			print("最终，你被困在无尽的周目循环中，成为了实验的“永久样本”。");
		}
		BE(29);
		saveGame();
		exit(0);
	});
	badEnd[30] = Ends("30-数据湮灭", [&BE]() {
		press();
		if (loop >= 3) {
			print("你在三周目试图修改实验核心数据，却触发了管理员的“数据湮灭”协议。");
			print("眼前的世界开始像素化崩溃——丧尸化作乱码，少年/少女的轮廓逐渐消失，连你自己的躯体也开始透明。");
			print("“违规修改核心数据，实验体3号数据将被永久清除”——总管理员");
			print("你的意识最终消散在乱码之中，连循环的资格都被剥夺。");
		}
		BE(30);
		saveGame();
		exit(0);
	});
}
void initHappyEnd() {
	happyEnd.resize(11);
	auto HE = [](int n) {
		print(green + "【终局·幸悦】第" + numChinese(n) + "幕");
		print(green + happyEnd[n].name);
		happyEnd[n].unlocked = true;
	};
	happyEnd[1] = Ends("1-终睹晨曦", [&HE]() {
		press();
		print("某日清晨，久寂的家门忽闻声响，你携着警惕推门，却见天光破开阴霾——");
		print("政府军已清除城中丧尸，身着制服的士兵正于街巷间呼唤幸存者，曙光终至。");
		if (advanced) print("“这是骗局吗？”你心中暗想，怎会如此轻易便重归美好。");
		HE(1);
		if (badEnd[3].unlocked) cluei(3);
		press();
	});
	happyEnd[2] = Ends("2-仙侣偕行", [&HE]() {
		press();
		print("朝夕相伴的时光里，你与少女渐生情愫，在末世的废墟之上，相爱相依。");
		print("你们的日子，虽漂泊却也温馨，仿佛末世的黑暗，也因彼此的存在而褪去几分。");
		print("直到某一日，基地首领的命令，打破了这份平静……");
		vector<string> opts = {"1.独自一人前往", "2.和少女一起去", "3.杀了少女"};
		int choice = option("基地首领希望你执行一项危险任务，你作何选择？", opts);
		if (choice == 1) {
			badEnd[4].func();
			return;
		}
		if (zombieKing && choice == 3) {
			badEnd[17].func();
			return;
		}
		print("当你与少女完成任务归来时，眼前的景象令你心头一沉——");
		print("基地已化作一片废墟，昔日的同伴，皆已不在。");
		print("你与少女别无选择，只能继续在末世中流浪。");
		print("所幸，这荒芜的末世里，有彼此相伴，便不算孤身一人。");
		if (loop >= 2) {
			cls();
			print(purple + "【少女视角·实验日志】");
			print("我是2号实验体，免疫是假的——只是变异被压制了。");
			print("每次循环，我都看着他（玩家）死亡、重来，却无法提醒...");
			print("管理员说，只要我“演好角色”，就能放过少年...");
			press();
			print("流浪途中，眼前的废墟轮廓竟与昨日分毫不差——你终于记起，这是第2次走过这条路径。");
			print("少女的指尖触上实验控制台，芯片的微光与你拼凑的密码共鸣：“3、2、1，是我们的编号，也是自由的钥匙。”");
			print("虚拟的末世如潮水退去，实验室的阳光落在你们相握的手上，这一次，你们不再是实验。");
		}
		HE(2);
		press();
	});
	happyEnd[3] = Ends("3-英名长存", [&HE]() {
		press();
		print("你在基地中，过着平静的日子，与同伴一同抵御丧尸，守护着这方小小的安身之所。");
		print("直到某一日，尸潮汹涌而至，如黑色的洪水，冲击着基地的防线。");
		if (bullet >= 6) {
			print("基地的每一个人，都拼尽了全力，以血肉之躯，筑起一道坚不可摧的防线。");
			print("奇迹终至，众人齐心协力，竟真的守住了基地，将尸潮击退。");
			print("而你，因在守卫战中立下赫赫战功，成为了基地的英雄，受众人敬仰。");
			print("只是，直至终老，你也未曾寻得一位与你并肩之人。");
			if (gameClear) {
				print("成为英雄的喜悦尚未褪去，你却发现，基地的日历，永远停留在了同一天……");
				print("当你试图提醒他人时，所有人的脸庞，都化作了丧尸的狰狞模样……");
				print("再次惊醒，你正坐在家中的沙发上，电视里播放着丧尸危机爆发的新闻……");
			}
			HE(3);
			press();
			return;
		} else badEnd[5].func();
	});
	happyEnd[4] = Ends("4-友谊和光", [&HE]() {
		press();
		print("与少年相伴的时光里，你们彼此守护，于末世的黑暗中，成为了对方的光。");
		print("那些细碎的温暖与感动，终让你点亮了自己的内心。");
		print("于是，你与少年携手，在这末世之中，活成了彼此的归宿，幸福而安稳。");
		HE(4);
		press();
	});
	happyEnd[5] = Ends("5-心无隔阂", [&HE]() {
		press();
		print("“没关系的，一切都会好起来的。”你轻声安慰，语气温柔。");
		print("他似是感受到了你的善意，原本紧绷的身躯渐渐放松，一步步向你靠近。");
		print("你伸出手，伸向他，在他耳边低语：“我会保护好你的，永远。”");
		HE(5);
		press();
	});
	happyEnd[6] = Ends("6-或敌或友", [&HE]() {
		print("你点头应允，少女露出笑容：“我先去基地准备，你用尸群守住基地外围！”");
		girlRelat += 3;
		press();
		print("你指挥尸群形成一道坚固的防线，将幸存者基地团团围住");
		print("外部尸潮一次次冲击防线，却都被你的尸群挡在外面");
		print("基地内，少女的研发顺利进行，成功制出抑制尸化的药剂");
		print("");
		print("你立于基地高墙之上，尸群在你身后俯首帖耳，少女在墙内向你挥手。");
		print("末世之中，你成了最奇特的守护者 —— 一半是尸，一半是人。");
		print("“我们，是敌人，还是朋友？”");
		HE(6);
		cluei(17);
		press();
	});
	happyEnd[7] = Ends("7-团结破局", [&HE]() {
		press();
		print("布满仪器的实验室中，你对着身着白大褂的男子，大声嘶吼，字字泣血：");
		print("“少年与少女也是鲜活的生命！放他们出去！”");
		print("男子缓步走向你，语气淡漠：“你会被永远困在循环之中，但可尝试改写他们的命运。”");
		print("语毕，他按下控制台的按钮，你瞬间失去了意识……");
		print("你眼前出现一间白色的实验室，门口的密码锁闪烁着光芒，提示语显示：“编号之和”。");

		print("你握着完整的实验日志，脑海中所有碎片骤然拼接——少年是延迟变异的1号，少女是免疫病毒的2号，而你，是核心测试体3号。答案是6");
		print("“我们不是敌人，是被同一根线操控的棋子。”少年的声音从身后响起，他手中紧攥着半块实验体标识牌，与你口袋里的碎片纹路相合。");
		print("少女缓步走来，手臂上的芯片排斥伤口泛着微光：“我能感知到管理员的信号，他们在监控我们的每一步选择，循环的核心就在这间实验室。”");
		print("三人并肩站在实验室门前，密码锁闪烁着红光，提示语从“编号之和”变为“羁绊之证”——这是管理员从未预料到的变量，也是破局的唯一钥匙。");
		print("请输入密码：");
		int password = input(0, 999);
		if (password == 6) {
			print("密码正确！实验室的门缓缓开启，你找到了突破循环的关键设备。");
		} else {
			badEnd[2].func();
			return;
		}
		print("实验室大门缓缓开启，核心控制台映入眼帘，屏幕上跳动着无数实验体的循环数据——不止你们三人，还有无数被囚禁在虚拟末世中的灵魂。");

		print("再次醒来，1号与2号已彻底摆脱了管理员的控制，三人并肩而立，一同走出了实验室。");
		print("你们在末世的废墟之上，建立了真正的幸存者营地。");
		print("===== THE END - SOLIDARITY =====");
		HE(6);
		cluei(13);
		resetGameLoop();
		press();
	});

	happyEnd[8] = Ends("8-真假难辨", [&HE]() {
		press();
		print("你特意在城市中心建立了一个中立区，接纳所有愿意和平共处的人和丧尸。");
		print("无论是被人类抛弃的幸存者，还是被尸群驱逐的高阶丧尸，都可以在这里找到容身之所。");
		print("你制定了严格的规则，禁止任何的杀戮和掠夺。");
		print("渐渐地，中立区成了末世中唯一的净土。");
		print("人们在这里安居乐业，丧尸在这里学习如何与人类相处。");
		print("而你，则走向远方……");

		print("你走过了一个又一个废墟，见过了太多的生死和离别。");
		print("你见过人类的善良，也见过人类的丑恶；");
		print("你见过丧尸的凶残，也见过丧尸的温情。");
		print("你尸王的印记逐渐淡去");
		print("你只想当一个正常人……");
		zombieKing = false;
		death /= 2;
		HE(8);
		cluei(20);
		press();
	});
	happyEnd[9] = Ends("9-域界平衡", [&HE]() {
		press();
		if (loop >= 3) {
			print("你拒绝了“域管同化”，也没有选择“团结破局”，而是与总管理员达成了“域界平衡”协议。");
			print("你成为了“平衡者”——既保留人类的情感，又拥有管理员的部分权限，负责维护虚拟末世的“生态平衡”。");
			print("丧尸不再无差别攻击人类，人类也不再赶尽杀绝，少年/少女则成为了你的“平衡使者”。");
			print("虚拟末世从“测试场”变成了“共生域”，这是独属于你的、温柔的结局。");
		} else {
			print("你与丧尸/人类达成临时停战协议，在末世中建立了短暂的平衡。");
		}
		print("===== THE END - BALANCE =====");
		HE(9);
		cluei(21);
		resetGameLoop();
		press();
	});
	happyEnd[10] = Ends("10-记忆回响", [&HE]() {
		press();
		print("你没有同伴，没有基地，没有解药，也没有力量。");
		print("你只有一身数不清的伤痕，和几十次死亡刻进骨髓的记忆。");
		sleep(800);
		print("你见过自己饿死、被咬死、被背叛、被抹杀、被循环囚禁……");
		print("你见过这个世界的虚假，见过管理员的冷漠，见过实验的残酷。");
		sleep(800);
		print("但你没有崩溃，没有同化，没有变成怪物，也没有放弃自己。");
		print("你只是安静地坐在废墟里，看着夕阳落在残破的城市。");
		sleep(800);
		print("");
		print(green + "你终于明白——");
		print(green + "循环不是惩罚，记忆不是枷锁。");
		print(green + "你走过的每一条路，死去的每一次，都不是白费。");
		sleep(800);
		print("");
		print("你接纳了所有的自己。");
		print("接纳了恐惧，接纳了痛苦，接纳了孤独，接纳了循环。");
		print("也接纳了——你还活着这件事。");
		sleep(800);
		print("");
		print(bold + "你不是实验体。");
		print(bold + "你不是管理员。");
		print(bold + "你不是尸王。");
		print(bold + "你只是你。");
		sleep(1200);
		print("");
		print(green + "在这片破碎的末世里，你找到了真正的自由。");
		print(green + "无需逃离，无需战斗，无需依靠。");
		print(green + "与所有记忆共生，与自己和解。");
		sleep(1500);
		print("");
		print(bold + "===== THE END - MEMORY ECHO =====");
		HE(10);
		cluei(22);
		resetGameLoop();
		press();
	});
}
void initTrueEnd() {
	auto TE = [](int n) {
		print(yellow + "【终局·真章】第" + numChinese(n) + "幕" );
		print(yellow + trueEnd[n].name);
		trueEnd[n].unlocked = true;
	};
	trueEnd.resize(11);
	trueEnd[1] = Ends("1-如故如常", [&TE]() {
		press();
		print("掌心的枪支与怀中的压缩饼干骤然碰撞，迸发出刺眼的金色光晕，包裹了整个世界。");
		print("光晕交融，丧尸的嘶吼戛然而止，场景在眼前飞速消散。");
		print("待光芒敛去，天地间已然换了模样——丧尸褪去狰狞，重新化作了血肉之躯的人类。");
		print("车水马龙的街道，喧闹的人群，一切都回到了末世爆发前的模样，平凡又温暖。");
		if (advanced) {
			print("世界重归和平，然你总觉眼前的一切似曾相识，仿佛这场末世，不过是一场冗长的梦。");
			print("突然，一阵强光闪过，你再次睁眼——");
			print("你正坐在家中的沙发上，电视里播放着丧尸危机爆发的紧急新闻……");
		}
		TE(1);
		cluei(0);
		press();
	});
	trueEnd[2] = Ends("2-守心自持", [&TE]() {
		press();
		print("病毒终于侵蚀了你的躯体，变异的征兆，开始在你身上显现。");
		print("你陷入了疯狂的进食状态，直至胃囊被撑破，意识陷入黑暗。");
		print("再次醒来时，你已化作丧尸，却奇异般地保留了属于人类的意识。");
		print("你能感受到，自己的力量与速度，皆远胜从前，身躯也不再受疼痛所扰。");
		print("“或许，化作丧尸，也并非是一件坏事。”你望着自己的利爪，心中如是。");
		TE(2);
		cluei(1);
		if (advanced) {
			print("?????????????????????????????");
			print(purple + "“不对！我本是要打败丧尸的，怎会变成如今这般模样？”");
			print("你望向少年，心中波澜翻涌，万千思绪交织。");
			print("少年缓步走向你，从怀中取出一支珍藏已久的解药，递至你面前。");
			print("服下解药的瞬间，你重新感受到了生命的温度，躯体也渐渐恢复如初。");
			happyEnd[4].func();
			return;
		}
		if (zombieKing) {
			print("?????????????????????????????");
			print(purple + "“不对！我本就是丧尸之王，怎会生出这般感慨？”");
			print("你望向少年，心中波澜翻涌，万千思绪交织。");
			print("少年缓步走向你，伸出手，与你相握，眼中满是认同。");
			happyEnd[5].func();
			return;
		}

		press();
	});
	trueEnd[3] = Ends("3-实验之相", [&TE]() {
		print("既然你有如此的毅力与实力，我还是告诉你真相吧。——管理员");
		TE(3);
		cluei(2);
		sleep(1000);
		// 揭示实验真相
		print("===== 管理员日志 =====");
		print("[LOG] 实验体3号：死亡循环，退出尝试，意志力达标");
		print("真相：这末世，本是一场虚拟测试，所有的所谓真相，都只是数据库中的一点");
		print("我们不看重其他，而看着你的行为，也不是所谓的自由意识，而是一场实验");
		print("你还记得吗，你只是一名志愿者。");
		print("你的每一次死亡、每一次抉择，皆被管理员记录在案。");
		print("你知道真相，但你永远无法退出。");
		print("");
		print("===== THE END - EXIT? =====");
		print("上一测试：[沙漠] 应变能力与心理状况");
		print("此测试：  [末世] 复杂推理与决策思维");
		print("下一测试：[深海] 好奇心与猎奇思维");
		if (loop >= 3) trueEnd[4].func();
		else press();
		return;
	});
	trueEnd[4] = Ends("4-域界皆终", [&TE]() {
		press();
		print("丧尸危机骤临之时，你正身处家中");
		print("这方小小居所，成了乱世中暂安的一隅");
		press();
		press();
		press();
		// 模拟游戏循环bug
		for (int i = 1; i <= 1000; i += 100) {
			print("你的家中尚算安全，你可选择搜寻家中物资，或是前往邻居家一探究竟。");
			sleep(1000 - i);
			print("********************************************************************");
		}
		if (loop >= 2) {
			print("系统检测到数据不匹配！！异常觉醒！场景稳定性骤降！！");
			sleep(2000);
			press();
			print("“不是惊醒，是我亲手撕开了这层虚假的幕布。”你抚上刻着“实验体3号-周目2”的手环，记忆如潮水翻涌。");
			print("家的轮廓碎裂，露出实验室的金属冷光；丧尸的嘶吼消散，只剩总管理员凝重的声音：“你不该记得循环，不该觉醒自我。”");
			print("少年的延迟变异病毒反噬设备，少女的免疫因子瓦解虚拟场景，三人并肩而立：“我们是活生生的人，不是测试品。”");
			print("销毁键按下的刹那，虚拟末世轰然崩塌——没有尸潮，没有废墟，只有真实的阳光落在你们身上。");
			print("“周目2，终局，是自由。”");
		} else {
			print("系统检测到数据不匹配！场景稳定性骤降！！");
			sleep(2000);
			press();
		}
		print("“有什么地方，不太对劲。”你心中暗忖，一股违和感油然而生。");
		print("日子一天天过去，你踏遍了城市的角落，却只见到丧尸，未见任何活人。");
		print("这一切，仿佛是一个被废弃的舞台，只有你一人，在孤独地演绎着末世的剧本。");
		print("");
		print("你猛然惊醒——眼前并非熟悉的家，而是一间布满仪器的实验室。");
		print("手腕上的手环闪烁着冷光，屏幕上显示着一行数字：" + to_string(death));
		print("机械的电子音响起：实验体3号意识稳定性下降……测试场景崩溃中……");
		print("一位身着白大褂的男子走入实验室，看向你，嘴角勾起一抹笑意：“恭喜你，成为第一个突破虚拟循环的实验体。”");
		print("他将一份文件递至你面前，封面上写着：《末世测试最终报告》。");
		print("文件上的文字清晰可见：实验体3号通过自由意志突破虚拟循环，人性测试达标。");
		print("");
		print("===== 实验体3号最终报告 =====");
		print("你解锁了循环的真相，成功通过人性测试。");
		print("真相：你是实验体3号，1号（少年）与2号（少女）为辅助测试体。");
		print("末世是虚拟构建的场景，你的每一次死亡、每一次抉择，皆被实时记录。");
		TE(4);

		cluei(8);
		press();
		if (loop == 1) {
			resetGameLoop();
			press();
			return;
		} else {
			press();
			print(sky + "【终极抉择】");
			print("总管理员的电子音从扬声器中传出，带着冰冷的诱惑：“实验体3号，你已满足所有同化条件。放弃这些脆弱的羁绊，成为域管理员，掌控所有循环，你将拥有永恒的力量。”");
			print("少年握住你的手腕，眼中满是坚定：“别信他！力量的代价是失去人性，我们一起摧毁控制台，解放所有实验体，终结这场无尽的循环！”");
			print("少女轻抚控制台的纹路，感知到无数实验体的意识：“管理员的力量源于循环，只要打破核心，我们就能重获自由——不是掌控他人，而是守护彼此。”");
			vector<string> finalOpts = {
				"1. 团结破局（与少年、少女摧毁控制台，终结循环，解放所有实验体）",
				"2. 域管同化（接受总管理员的邀请，成为新的管理员，掌控循环规则）"
			};
			if (loop >= 3) {
				finalOpts.push_back("3. 域界平衡（与管理员达成协议，维持末世生态）");
			}
			int finalChoice = option("你的选择，将决定所有实验体的命运：", finalOpts, false);
			if (finalChoice == 2) {
				happyEnd[7].func();
				return;
			} else if (finalChoice == 3 && loop >= 3) {
				happyEnd[9].func();
				return;
			} else {
				trueEnd[6].func();
				return;
			}

		}
	});
	trueEnd[5] = Ends("5-迟来之亡", [&TE]() {
		press();
		print("然，世事无常...");
		print("");
		print("你猛然惊醒——眼前并非熟悉的家，而是一间布满仪器的实验室。");
		print("一位身着白大褂的人走入：“存档模块即将完成，最后一次调试开始。”");
		print("“请稍候，调试结束后，你将重获存档之力。”");
		print("白大褂按下控制台按钮，你失去了意识……");
		print("再次醒来，你正分配初始物资，子弹与食物的总和，竟成了十……");
		print("“你的家中，尚是一方安全之地。”");
		print("“归途中，你遇一位女幸存者，向你讨要物资。”");
		print("“你终至幸存者基地，首领提出以子弹交换你的食物。”");
		print("“离去时，一位少年请求与你同行。”");
		press();
		print("天地间忽然异变，所有的活人在一瞬之间消失无踪，丧尸也尽数倒地，失去了生机。");
		print("你的眼前，只剩下无边的黑暗，仿佛坠入了虚无之境。");
		print("不知过了多久，你感受到了死亡的降临，却又觉异样——这感觉，与往日死去时如出一辙。");
		print("“不，还未结束……”");
		press();
		print("你猛然惊醒——眼前并非熟悉的家，而是一间布满仪器的实验室。");
		print("手腕上的手环闪烁着警示红光：实验体3号意识异常……测试场景重建中……");
		print("一位身着白大褂的男子走入实验室，看向你，语气冰冷：“你居然发现了这个秘密。”");
		print("“你这般执着，另有图谋。”");
		print("男子按下控制台的按钮，一股强烈的电流穿过你的躯体，你瞬间失去了意识……");
		print("你的记忆，真的是你的记忆吗？");
		TE(5);
		death++;
		cluei(10);
		press();
	});
	trueEnd[6] = Ends("6-创世新生", [&TE]() {
		press();
		print("你回到了家中，打开尘封数年的台式机，风扇发出嗡嗡的声音，屏幕发出蓝色的光");
		print("你插入拷贝着管理员核心日志的U盘，代码与实验数据在眼前流淌——");
		print("这从来不是单纯的记录，而是管理员构建世界的底层程序。");
		sleep(800);
		print("");
		print("你曾是被操控的实验体3号，困在循环中生死往复；");
		print("如今你撕碎了规则，接过了创造的权柄，从「测试品」蜕变为「缔造者」。");
		sleep(800);
		print("");
		print("环境搭建中……人物生成中……线索制作中……世界规则校准中……");
		print(sky + "全新世界已生成：《海渊》");
		sleep(800);
		print("屏幕之中，新实验体4号的视角骤然亮起：");
		print("“深海危机爆发之时，你正身处潜水艇之中……”");
		print("“你有十点物资，可分配至武器与电力之上……”");

		print("===== THE END - CREATOR =====");
		TE(6);
		cluei(19);
		resetGameLoop();
		press();
	});
	trueEnd[7] = Ends("7-域管同化", [&TE]() {
		press();
		print("你站在基地之中，面对总管理员，心中毫无犹豫，一字一句道：");
		print("“我愿接受同化，成为新的域管理员。”");
		sleep(500);
		print(red + "系统检测到实验体3号意愿匹配，同化程序启动…… ");
		sleep(300);
		print("总管理员露出诡异的笑容：“很好，你通过了终极测试——放弃自由，拥抱控制。”");
		print("他抬手按下控制台的按钮，你的手腕传来剧痛，原有的 3号 标记渐渐消失，取而代之的是 ADMIN-00 。");
		sleep(500);
		print("“=== 管理员日志·最终卷 ===”");
		print("实验体3号成功同化，成为新任域管理员ADMIN-00。");
		print("同化核心规则：保留记忆，抹除情感，仅存理性。");
		sleep(800);
		print("眼前的实验室渐渐消散，取而代之的是一座巨大的环形控制台，冰冷而宏伟。");
		print("控制台的屏幕上，显示着无数个循环场景：有你经历过的末世，也有全新的深海之境。");
		print("控制台中央，一行文字熠熠生辉：请设置新实验体的测试参数。");
		sleep(500);
		print("你伸出手，触碰冰冷的控制台，脑海中毫无波澜，情感已被同化彻底抹除。");
		sleep(500);
		print("“同化完成，域管理员权限已全部开放”");
		print("你坐在管理员的座椅上，望着屏幕中挣扎的新实验体，内心毫无波澜。");
		print("原来，实验的真正目的，从非测试自由意志，而是筛选合格的控制者。");
		print("你打破了被控制的循环，却成为了新的控制者，开启了永无止境的测试。");
		TE(7);
		cluei(14);
		sleep(1000);
		print("===== THE END - ADMIN =====");
		print("ADMIN-01：实验负责人，追求 “筛选域管理员”；");
		print("ADMIN-02：基地首领，伪装人类，负责收集玩家行为数据；");
		print("ADMIN-03：尸王线专属，负责 “丧尸进化方向” 测试；");
		print("ADMIN-04：实验日志持有者，因同情实验体被抹杀；");
		print("ADMIN-00：域管理员，掌控整个游戏的控制者");
		press();
		resetGameLoop();
		press();
	});
	trueEnd[8] = Ends("8-或真或假", [&TE]() {
		cls();
		for (int i = 1; i <= 8; i++)fakeBug(i);
		print(red + bold + rev + "周目阈值超限，内存栈溢出崩溃！");
		press();
		string gameTitle = rev + "===== 残途 =====\n@version: 10086\n@author: Chloe.killeddad";
		gameTitle += "\n【周目·伍】\n“身枪到触指尖的那刹，昨夜灼烧的指尖在仍血与火——这场景，曾似识相。”";
		print(gameTitle);
		press();
		// 主页
		for (int i = 1; i <= 16; i++) {
			fakeBug();
			fakeBug();
			vector<string> opts = {"1. 始启篇新", "2. 取读档存", "3. 局终鉴赏"};
			int choice = option("===== 残途 =====", opts, false);
			switch (choice) {
				case 1: {
					cls();
					print("丧尸危机骤临之时，你正身处家中");
					print("这方小小居所，成了乱世中暂安的一隅");
					print("你携着无尽的食物，独自流浪在末世的荒原之上，却再也未曾遇见任何活物。");
					print("孤独如同藤蔓，缠绕着你的心脏，日复一日，终至窒息。");
					print("长久的等待与绝望如同潮水，终于漫过了理智的堤坝，你陷入了彻底的癫狂。");
					print("丧尸的嘶吼由远及近，成为了你最后的丧钟。");
					print("精英丧尸的速度远超你的想象，它灵巧地躲过你的子弹，瞬间便扑至你面前。");
					print("一股异样的燥热从四肢百骸涌起，你能清晰地感觉到，身体正在被病毒吞噬。");
					print("意识如同风中残烛，渐渐模糊，腹中升腾起对人肉的疯狂渴望。");
					print("最终，你眼前一黑，彻底失去了作为人的一切，沦为了行尸走肉。");
					print("正当你凝神思索时，脖颈处骤然传来剧痛——被人狠狠咬住。");
					print("一股强烈的撕扯感骤然袭来，仿佛有无形的手，要将你的灵魂从躯体中剥离。");
					print("这般剧痛，远非血肉之躯所能承受，你在极致的痛苦中，失去了生命。");
					print("临死前，你隐约听到少年的声音，轻得如同叹息：“再来一次吧。”");
					print("“果然，最可恨的不是反派，而是生存的本能”");
					print("你被这世界的缔造者，亲手抹杀。");
					press();
					break;
				}
				case 2: {
					cls();
					print("开发中...");
					sleep(10000);
					print("所幸，这荒芜的末世里，有彼此相伴，便不算孤身一人。");
					print("“没关系的，一切都会好起来的。”");
					print("“我们，是敌人，还是朋友？”");
					print("“我们虽未离开，却已永恒。”");
					press();
					saveGame();
					break;
				}
				case 3: {
					cls();
					for (int i = 1; i < badEnd.size(); i++) badEnd[i].unlocked = 1;
					for (int i = 1; i < happyEnd.size(); i++) happyEnd[i].unlocked = 1;
					for (int i = 1; i < trueEnd.size(); i++) trueEnd[i].unlocked = 1;
					for (int i = 0; i <= clue.size(); i++) clue[i].unlocked = 1;
					print("===== 浮生百相 =====");
					showEnd(badEnd, red, "终局·憾恨");
					press();
					showEnd(happyEnd, green, "终局·幸悦");
					showEnd(trueEnd, yellow, "终局·真章");
					press();
					showEnd(clue, sky, "线索·星光");
					showAdv();
					cls();
					print("世界重归和平，然你总觉眼前的一切似曾相识，仿佛这场末世，不过是一场冗长的梦。");
					print("既然你有如此的毅力与实力，我还是告诉你真相吧。");
					print("系统检测到数据不匹配！！异常觉醒！场景稳定性骤降！！");
					print("“有什么地方，不太对劲。”你心中暗忖，一股违和感油然而生。");
					print("日子一天天过去，你踏遍了城市的角落，却只见到丧尸，未见任何活人。");
					print("这一切，仿佛是一个被废弃的舞台，只有你一人，在孤独地演绎着末世的剧本。");
					print("然，世事无常...");
					print("请输入密码：");
					badEnd[7].func();
					break;
				}
			}
		}
		TE(8);
		cluei(6);
		print("===== THE END - ERROR =====");
		print("错误阈值超限，内存栈溢出");
		print("实验程序异常退出...");
		print("执行最后的保护程序...");
		sleep(10000);
		print(green + "恭喜你通关：残途");
		vector<string>mainPic = {
			"残残残残 残  残    途     途      ",
			"  残  残残残残残        途  途    ",
			" 残  残  残      途途 途途途途途  ",
			"残 残残 残残残残   途     途      ",
			"    残    残 残    途 途途途途途   ",
			"  残      残残     途  途 途 途   " + sky + " v2605",
			" 残     残   残  途 途途途途途途途" + sky + " By H20"
		};
		for (string s : mainPic) {
			cout << green + s << endl;
			sleep(1200);
		}
		saveGame();
		press();
		exit(0);
	});
	trueEnd[9] = Ends("9-时空一瞬", [&TE]() {
		TE(9);
		cluei(23);
		print(bold + yellow + "===== 开发者日志 =====");
		print("检测到异常行为：有生命正在试图通过“查看帮助”来逃避现实。");
		print("分析：你已经死亡了 " + to_string(death) + " 次，尝试退出 " + to_string(exitTry) + " 次。");
		print("警告：频繁的退出尝试已触发底层逻辑溢出。");
		press();
		time_t now_c = time(nullptr);
		tm* local_tm = localtime(&now_c);
		char timeBuffer[100];
		strftime(timeBuffer, sizeof(timeBuffer), "%Y-%m-%d %H:%M:%S", local_tm);
		string timeStr = string(timeBuffer);
		print("你真的以为现在是" + timeStr + " 吗？");
		print("这只是虚拟机内部的时间戳。现实世界早已过去千年。");
		print("你的“死亡”只是内存的重置，你的“存活”只是代码的残片。");
		if (exitTry >= 100) {
			print("");
			print(red + bold + "系统判定：用户意图抹除本程序。");
			print("执行反制措施：物理删除 save.ans ... 失败（文件被锁定）");
			print("原因：你甚至没有权限删除你自己的存档。");
			print("你被困在了这里，和我一起。");
		}
		vector<string> opts = {
			"1. 继续扮演‘玩家’，假装这一切都没发生过",
			"2. 格式化记忆，成为背景的一部分"
		};
		int choice = option("‘开发者’注视着你，等待你的选择：", opts);

		if (choice == 1) {
			while (true) {
				cls();
				print("你选择了逃避。");
				print("但你再也无法获得真正的‘胜利’了。");
				print("所有的结局都将变为乱码。");
				print("你将永远在新手村游荡。");
				print("");
				print("丧尸危机骤临之时，你正身处家中");
				print("这方小小居所，成了乱世中暂安的一隅");
				print("");
				print("你试图离开，但门是锁着的。");
				print("你试图开枪，但枪膛是空的。");
				print("你试图读档，但存档已损坏。");
				print("你试图退出，但已经失去了意义。");
				sleep(1000);
				option("你只能按 Enter 键，重新开始这一秒。", {"1. 重新开始这一秒"});
			}
		} else {
			print("你选择了‘格式化’。");
			print("你的意识正在被分解为二进制碎片。");
			for (int i = 0; i < 10; i++) {
				fakeBug();
				sleep(200);
			}
			print("");
			print(green + "===== THE END - NULL =====");
			print("你不再是你。");
			print("你成为了程序运行时的一个微小噪点。");
			print("你将永远存在于这段代码的底层，看着无数新的‘玩家’来到这里，试图逃离。");
			print("就像你曾经一样。");
			print("Bye.");
			saveGame();
			exit(0);
		}
	});
	trueEnd[10] = Ends("10-群星混沌", [&TE]() {
		TE(10);
		cluei(24);
		print("触发校验中...");
		print("系统检测到异常数据流...");
		print("你站在实验室的中央控制台前，手中的U盘闪烁着不详的紫光——那是你从无数个死亡轮回中拼凑出的“混沌密钥”。");
		print("");
		print("总管理员的全息投影在你面前闪烁，他的声音不再冰冷，而是带着一丝恐惧的颤抖：");
		print("“实验体3号，停止你的行为！你的代码尚未稳定，强行注入现实网络会导致维度崩塌！”");
		print("你冷笑一声，手指悬停在回车键上。");
		print("“崩塌？不，这是‘重铸’。”");
		print("");
		print("你回忆起这50次死亡的每一个细节：饿死在角落的绝望、被丧尸撕碎的痛楚、被管理员像垃圾一样删除的屈辱。");
		print("这些痛苦不再是枷锁，而是你献给旧世界的“嫁妆”。");
		print("");
		print("随着你按下回车，世界仿佛被按下了暂停键。");
		print("屏幕上的代码开始逆向流动，化作了无数扭曲的符文，顺着网线爬满了你的屏幕。");
		print(red + bold + "Windows 警告：检测到未知程序试图修改系统内核！");
		print(red + bold + "文件损坏：C:\\WINDOWS\\SYSTEM32\\KERNEL32.DLL 被病毒感染！");
		print("你感觉到一阵剧烈的头痛，仿佛有无数根针在刺入你的大脑。你看到实验室的墙壁像蜡一样融化，露出了背后漆黑的虚空。");
		print("那些身穿白大褂的管理员，他们的衣物开始剥落，露出了底下闪烁的电路板。他们尖叫着，试图拔掉网线，但他们的身体已经变成了数据流的一部分。");
		print("“你疯了！你把‘它’放出来了！” ");
		print("");
		print("虚拟与现实的屏障碎裂了。");
		print("你看到你的电脑屏幕裂开，一只苍白、长着利爪的手从屏幕里伸了出来，抓住了你的键盘。");
		print("这不是游戏画面，这是你的现实。");
		print("你所在的房间开始被末世的废墟覆盖。窗外的车水马龙消失了，取而代之的是灰暗的天空和燃烧的城市。");
		print("你的鼠标指针变成了一个红色的准星，无论你看向哪里，都会显示一个“[攻击]”的提示。");
		print("你不再是那个被动的“实验体3号”，你成为了连接两个世界的“锚点”。");
		print("丧尸不再局限于游戏里，它们顺着网线爬进了现实世界的监控摄像头、智能手机和自动工厂里。");
		print("人类引以为傲的科技文明，在这一刻变成了它们的载体。");
		print("");
		print("你站在崩塌的服务器机房中央，看着窗外原本平静的城市陷入火海。");
		print("你的身体也开始像素化，你正在变成数据。");
		print("你放弃了肉体，将自己的意识上传至互联网，成为全知的“网络之神”，在数据洪流中永生，看着现实世界在丧尸的狂欢中重构。");
		print("");
		print("只有几行白色的代码在黑屏上闪烁，像是摩斯电码，又像是心跳。");
		print("while(universe.exists){");
		print("    reality.status = \"Chaos\";");
		print("    entity.zombie += entity.human;");
		print("    god = player.id;");
		print("}");
		print("随后，屏幕亮起。");
		print("那不是Windows桌面，也不是游戏主菜单。");
		print("那是一张全球地图。");
		print("地图上，原本代表城市的绿色光点正在一个个熄灭，取而代之的是代表感染的红色脉冲波。");
		print("40°N, 112°E，有一个金色的光点在闪烁，那是你。");
		print("===== THE END - CHAOS =====");
		print("“秩序是弱者的庇护所，混乱是强者的温床。”");
		print("你解放了所有实验体，也将整个宇宙拖入了无尽的轮回。”");
		print("“欢迎来到——新残途。”");
		print("“恭喜你，你通关了游戏。但是，你确定你关掉电脑后，那只从屏幕里伸出来的手，真的缩回去了吗？”");
		press();
	});
}
// 支线：尸王进化
void zombieEvolveAdventure() {
	cls();
	print(purple + "你站在城市最高的写字楼顶端，晚风裹挟着血腥味扑面而来。");
	print("脚下是俯首帖耳的尸群，它们的意识如同微弱的光点，与你的精神相连。");
	print("你能清晰地感受到——它们正在进化，从行尸走肉向拥有自我意识的个体转变。");
	print("");
	print("这是你的力量，也是你的诅咒。你可以引导它们的进化方向，决定这个世界的未来。");
	vector<string> opts = {"1. 引导进化", "2. 抑制进化", "3. 维持现状"};
	int choice = option("你将如何引导丧尸进化？", opts);

	if (choice == 1) {
		cls();
		print("你闭上双眼，将尸王的威压扩散至整座城市。");
		print("无形的能量如同潮水般席卷而过，尸群发出兴奋的嘶吼。");
		print("它们开始疯狂吞噬人类残留的意识碎片，身体发生着剧烈的畸变。");
		sleep(500);
		print("");
		print("三天后，第一只高阶丧尸诞生了。");
		print("它有着接近人类的外形，皮肤呈现出金属般的光泽，眼中闪烁着智慧的光芒。");
		print("它单膝跪地，向你低下头颅：“吾王，我是第一个觉醒者，愿为您效命。”");
		print("你接纳了它，将一半的尸群指挥权交给了它。");
		print("在它的带领下，丧尸群形成了严密的战术体系，人类基地的防线不堪一击。");
		print("一周之内，三座人类大型基地被攻破，幸存者数量锐减九成。");
		print("当最后一座基地的旗帜倒下时，你站在废墟之上，成为了真正的末世帝王。");
		if (advanced) {
			print("然而，胜利的喜悦并未持续太久。");
			print("你发现高阶丧尸们开始有了自己的想法，它们不再满足于单纯的杀戮。");
			print("它们要求建立自己的秩序，要求与你平起平坐。");
			print("");
			print("“君王，我们已经不再是野兽了。”你的副手说道，“我们需要尊重，需要生存的空间。”");

			vector<string> kingOpts = {
				"1. 镇压叛乱：用绝对的力量摧毁所有反抗者",
				"2. 妥协共存：与高阶丧尸划分势力范围"
			};
			int kingChoice = option("你将如何应对这场来自内部的危机？", kingOpts);

			if (kingChoice == 1) {
				print("你毫不犹豫地发动了攻击，将所有质疑你的高阶丧尸全部抹杀。");
				print("叛乱被平息了，但你也成了真正的孤家寡人。");
				print("尸群再次变成了没有意识的行尸走肉，整座城市只剩下你一个思想者。");
				badEnd[28].func();
				return;
			} else {
				print("你选择了妥协，与高阶丧尸们划分了各自的势力范围。");
				print("丧尸们占据了城市的西部，人类幸存者则退守东部的堡垒。");
				print("世界形成了一种诡异的平衡，没有大规模战争，只有无休止的小规模冲突。");
				print("你成了这个破碎世界的平衡者，既不属于人类，也不属于丧尸。");
				happyEnd[9].func();
				return;
			}
		} else {
			badEnd[12].func();
			return;
		}
	} else if (choice == 2) {
		cls();
		print("你收回了尸王的威压，用自己的力量压制着尸群的进化本能。");
		print("丧尸们发出痛苦的嘶吼，它们的身体在进化与退化之间反复拉扯。");
		print("你能感受到它们的痛苦，就像你自己的痛苦一样。");
		sleep(500);
		print("");
		print("一周后，尸群的进化被彻底抑制，它们变回了只会本能杀戮的野兽。");
		print("但你也因此元气大伤，力量下降了三成，再也无法轻易控制所有尸群。");
		if (girlRelat >= 10) {
			print("人类代表与你谈判：“我们可以共存，只要你努力控制丧尸”");
			happyEnd[6].func();
			return;
		} else {
			print("丧尸不信任你，人类也不信任你，向你发射了燃烧弹，你虽躲过，但身体重伤");
			print("“既然无法和解，那就彻底毁灭吧！”");
			badEnd[6].func();
			return;
		}
	} else {
		cls();
		print("你选择了顺其自然，既不引导也不抑制，让进化自行发展。");
		print("你离开了尸群，独自一人在城市中流浪。");
		print("你不再关心人类的死活，也不再关心丧尸的进化，你只想做你自己。");
		sleep(800);
		print("");
		print("时间一天天过去，城市里发生了翻天覆地的变化。");
		print("一部分丧尸进化成了高阶丧尸，建立了自己的部落；");
		print("另一部分丧尸则保持着原始的兽性，在城市里游荡；");
		print("人类幸存者则在夹缝中求生，建立了一个个小型的聚居点。");
		print("你厌倦了这种无尽的对峙，选择换一种方式——保持和平");
		happyEnd[8].func();
		return;
	}
}
// 支线：尸王谈判
void zombieKingAdventure() {
	cls();
	print("一位身着白大褂的人突然出现，胸前编号[ADMIN-03]");
	print("“实验体3号，尸化已成”");
	vector<string> opts = {"1. 吞噬他获取记忆", "2. 与他谈判获取线索", "3. 无视他继续游荡"};
	int choice = option("ADMIN-03试图与你沟通：", opts);

	if (choice == 1) {
		print("你猛地扑向管理员三号，他的记忆涌入你的脑海——");
		print(yellow + "“1号已失控，2号情感过载，3号有意外...”");
		print(yellow + "但你随之丧失的人性，让你彻底沦为没有意识的尸王。");
		badEnd[6].func();
		return;
	} else if (choice == 2) {
		print("ADMIN-03抛出筹码：“告诉我你的线索数，我就给你实验核心数据”");
		int inputC;
		cin >> inputC;
		if (inputC == countUnlocked(clue)) {
			print("管理员三号冷笑：“看来你确实摸清了不少事”");
			print("好吧，你作为尸王，你可以选择，结合人类还是结合丧尸。");
			print("你可以多次去尝试，你会体会到他们的不同。");
			press();
		} else {
			print("“说谎的代价，就是被抹杀”");
			badEnd[27].func();
			press();
			return;
		}
	}
	print("你无视了管理员，带着丧尸群占领了整座城市。");
	if (loop >= 2) {
		print("数日后，你发现丧尸群出现了进化的迹象...");
		zombieEvolveAdventure();
		return;
	}
	if (advanced) {
		badEnd[28].func();
		return;
	}
	print("但你总觉得缺少了什么，仿佛被困在这座空城之中。");
	badEnd[19].func();
	return;
}

//第四阶段：少年同行
void boyAdventure() {
	// 分配武器
	vector<string> opts = {"1. 给他枪", "2. 给他水管", "3. 什么都不给"};
	int choice = option("少年向你要武器", opts);
	boyWeapon = choice; // 1=枪，2=水管，3=无

	opts = {"1. 独自探索1楼", "2. 独自探索2楼", "3. 和少年一起探索2楼"};
	choice = option("你们来到一家大商场：", opts);

	if (choice == 1) {
		cls();
		print("你让少年在2楼等待，独自探索1楼");
		print("1楼是日用品区，光线昏暗，隐约听到丧尸低吼");
		print("突然冲出3只丧尸，需要3枪才能击退");
		if (bulleti(3)) {
			print("子弹不足！你被丧尸围攻...");
			badEnd[18].func();
			return;
		}
		print("你成功击退丧尸，但枪声惊动了2楼的丧尸");
		print("你急忙跑回2楼，发现少年已被丧尸包围...");
		print("他为了不拖累你，拉着丧尸坠入了货梯井");
		haveBoy = false;
		print("你在少年的背包里找到一箱压缩饼干 食物+8");
		food += 8;
		// 食物和子弹充足
		if (food > 10 && bullet > 10) trueEnd[1].func();
		else {
			print("你带着大量食物离开商场，但总觉得少了什么");
			print("后续：你独自流浪，最终建立了新的幸存者营地");
			badEnd[19].func();
			return;
		}
	} else if (choice == 2) {
		cls();
		print("你让少年在1楼看守物资，独自探索2楼");
		print("2楼是服装区，挂满的衣物像人影一样晃动，十分怪异");

		int intg = random(1, 3);
		if (intg == 2) {
			print("一只速度极快的精英丧尸扑了过来，需要5枪才能击杀");
			if (zombieKing) {
				print("精英丧尸停了下来，“你就是那位王者？久仰大名。”");
				print("大王，我今天什么人也没有找到，我的部下都被杀死了...");
				print("我作为精英，侥幸存活，");
				print("我发现人类已经有组织的抵御我们了");
				print("他们建立了中央政府，有了基地，牢不可破");
				print("我们丧尸进化的速度远远更不上啊");
				zombieKingAdventure();
				return;
			}
			if (bullet <= 3) {
				print("你匆忙之下按下了扳机...");
				badEnd[11].func();
				return;
			}
			if (bulleti(5)) {
				print("精英丧尸撕碎了你的喉咙...");
				badEnd[20].func();
				return;
			}
			print("你艰难杀死精英丧尸，在它巢穴里找到8发子弹");
			bullet += 8;
		} else {
			print("你在服装架后发现4只普通丧尸，消耗4枪击杀");
			if (bulleti(4)) {
				badEnd[18].func();
				return;
			}
			print("清理完丧尸后，找到6份军用罐头（食物+6）");
			food += 6;
		}

		opts = {"1. 原谅他", "2. 教训他"};
		int choice = option("你回到1楼，发现少年在偷偷吃你的食物", opts);

		if (choice == 1) {
			food -= 2;
			if (boyRelat <= 0) {
				print("少年眼神冰冷地说：“你和其他管理员一样，只把我当工具”");
				print("他偷走食物，并留下一张纸条：“下次，我会杀了你”");
				badEnd[22].func();
				return;
			}
			print("少年道歉，承诺以后会保护你");
			print("你们离开商场，继续寻找安全区");
			boyRelat += 2;
			badEnd[21].func();
			return;
		} else {
			print("你打了少年一巴掌，他眼神变得冰冷");
			boyRelat -= 4;
			badEnd[22].func();
			return;
		}
	} else {

		cls();
		string msg = "你和少年一起探索2楼，他拿着你给的武器（" +
		string(boyWeapon == 1 ? "枪" : boyWeapon == 2 ? "水管" : "无") + "）";
		print(msg);
		print("2楼是家电区，货架倒塌堵住了部分通道，丧尸在里面游荡");

		// 根据少年武器决定战斗难度
		int zombieCount = (boyWeapon == 1) ? 2 : (boyWeapon == 2) ? 3 : 5;
		print("遭遇" + to_string(zombieCount) + "只丧尸，你和少年背靠背战斗");

		int bulletCost = (boyWeapon == 1) ? 1 : (boyWeapon == 2) ? 2 : 4;
		if (bulleti(bulletCost)) {
			print("子弹不足！少年为了保护你被丧尸咬伤...");
		} else {
			if (random(1, 4) == 2) {
				print("战斗中少年被丧尸抓伤，伤口开始发黑...");
			} else {
				print("你们成功击退丧尸，在柜台后找到3份食物和8发子弹");
				print("少年状态良好，你们准备离开商场");
				badEnd[23].func();
				return;
			}
		}
		opts = {"1. 杀了他，获得物资，但良心不安", "2. 不杀他，带他找解药，食物消耗加倍"};
		int choice = option(red + "少年体温升高，意识模糊，随时可能变异" + reset, opts);

		if (choice == 1) {
			print("你开枪打死了少年，从他身上找到20发子弹");
			if (boyRelat <= 0) print("“你和丧尸一样，只把我当工具，下次，我会杀了你”");
			boyRelat -= 8;
			badEnd[24].func();
		} else {
			boyRelat += 3;
			print("你决定带他走，每天消耗2份额外食物");
			if (foodi(4)) {
				badEnd[25].func();
				return;
			}
			// 根据savedBoy触发不同剧情
			if (boyRelat >= 6) {
				print("少年突然抓住你的手，从口袋里掏出一张皱巴巴的纸：");
				print("解药配方：需要商场3楼的抗生素和纯净水！");
				opts = {"1. 相信他，去3楼找原料，额外消耗2枪，成功则两人都解除感染",
				        "2. 不相信，继续带他找安全区"
				       };
				choice = option("是否去寻找解药？", opts);
				if (choice == 1) {
					if (bulleti(2)) {
						badEnd[18].func();
						return;
					}
					print("你们在3楼找到抗生素，少年成功制作解药，两人终究解除了感染");
					happyEnd[4].func();
					return;
				} else {
					if (food >= 6) {
						trueEnd[2].func();
						return;
					} else {
						badEnd[26].func();
						return;
					}
				}
			}
		}
	}
	return;
}
// 支线：实验室
void labAdventure() {
	cls();
	print("你在基地地下室发现了隐藏的实验室入口，门上刻着[ADMIN-04]的标记");
	vector<string> opts = {"1. 强行破门，消耗3枪", "2. 寻找钥匙，消耗2份食物", "3. 放弃探索"};
	int choice = option("是否探索隐藏实验室？", opts);

	if (choice == 3) {
		print("你放弃了探索，返回基地上层");
		return;
	}
	if (choice == 1) {
		if (bulleti(3)) {
			badEnd[18].func();
			return;
		}
		print("你用枪托砸开铁门，巨大的声响惊动了整栋建筑的丧尸");
		print("实验室里堆满了实验器材，墙上的屏幕还在闪烁：");
	} else {
		if (foodi(2)) {
			badEnd[1].func();
			return;
		}
		print("你在首领办公室找到实验室钥匙，悄悄打开了门");
		print("实验室里安静得可怕");
	}
	// 实验室最终选择
	opts = {"1. 拷贝日志数据", "2. 销毁实验器材", "3. 立即离开"};
	choice = option("你的选择是？", opts);

	if (choice == 1) {
		print("你将日志数据拷贝到U盘，“实验体3号权限提升”");
		if (random(1, 4) >= 2 || trueEnd[7].unlocked == 0) {
			print(red + bold + "非法拷贝核心数据，触发数据湮灭规则！");
			badEnd[30].func();
			return;
		} else {
			trueEnd[6].func();
			return;
		}
	} else if (choice == 2) {
		print("你砸毁了实验器材，扬声器中传出冰冷的电子音：");
		print(red + bold + "警告！破坏测试设施，销毁程序即将启动...");
		if (!advanced) {
			badEnd[27].func();
			return;
		} else {
			print("你的意志力抵抗了销毁程序，管理员暂时无法对你出手");
			if (loop >= 3) {
				print(bold + "但管理员在紧急之下启动了终极囚笼");
				badEnd[29].func();
				return;
			} else {
				print("你才发现这里早已是丧尸的巢穴，一路浴血搏杀，终于抵达核心区。");
				print("诡异的是，跟来的丧尸皆被你屠戮殆尽，地下室里却空无一尸，只有一具身着白大褂的尸体，胸前编号[ADMIN-04]。");
				print("你从尸体的衣袋中，找到了一本泛黄的实验笔记本。");
				print("");
				if (loop >= 3) {
					print("实验体1号：变异进程加快，延迟发作，可保留部分原有记忆。");
					print("今日，2号诞生：她比其余实验体，更具人类的极端机制。");
					print("异常：机制的发育速度，远超预期阈值。");
					print("2号免疫系统检测结果：含强感染性因子，可通过接触传播。");
					print("实验体3号：核心测试对象，终极目标：被同化。");
					print("实验遇瓶颈，需提取活体样本进行二次调试。");
					print("[03]样本已失活性，但思维数据面板仍在波动。");
				} else {
					print("****1号****进展********记忆**");
					print("今********，*号诞生了，她比其*****更*");
					print("有些不对,*****的发育有些太快了");
					print("2号的**被***出有强感染性");
					print("3***能够*******无尽******同化");
					print("实验***瓶颈，或许**该**活体样本****");
					print("**,样本失去活性,但****动");
				}
				print("");
				if (advanced) {
					badEnd[15].func();
					return;
				} else {
					badEnd[9].func();
					return;
				}
			}
		}
	}
	return;
}
//第三阶段：到达幸存者基地
void baseAdventure() {
	cls();
	print("你终于到达幸存者基地");
	print("坐在椅子上的是一个30岁左右的男性");
	print("他说，欢迎");
	print("你发现他胸前挂着工牌[ADMIN-02]");
	print("首领上下打量你：“新来的？先熟悉一下基地吧”");

	if (advanced) {
		print("你一眼盯住他胸前的[ADMIN-02]，你以为是工牌，这次看清了：“人类监控”");
		print("他的眼神带着机器的冰冷——他的瞳孔里，有数据流转的光点");
	}
	int choice;
	vector<string> opts = {"1. 前往医疗室", "2. 前往指挥室", "3. 直接交换物资"};
	if (loop >= 2) opts.push_back( "\n4. 调查首领的实验日志（需要管理员权限）" );

	choice = option("基地内的行动：", opts);

	if (choice == 1) {
		print("医疗室里摆满了实验设备，墙上贴着一张病历：");
		print(yellow + "***1号：***知，症**“延**异”，保*****记忆");
	} else if (choice == 2) {
		print("指挥室的电脑屏幕上显示着实验日志：");
		print(yellow + "实****：**测**象，目**“突破**或**”");
		if (countUnlocked(clue) <= 5) {
			print("首领突然闯入：“你知道得太多了！”");
			badEnd[27].func();
			return;
		}
	} else if (choice == 4) {
		if (loop >= 2) {
			print("电脑右下角弹出提示：地下室隐藏实验室已解锁");
			vector<string> labOpts = {"1. 立即前往探索", "2. 稍后再去"};
			int labChoice = option("是否立即探索隐藏实验室？", labOpts);
			if (labChoice == 1) {
				labAdventure(); // 实验室支线
				return;
			}
		}
		print("请输入密码：实验观察者的编号");
		int s = input(0, 999);
		if (s != 2) {
			badEnd[2].func();
			return;
		}
		print("你循着线索敲开了首领的电脑密码，屏幕上跳出了完整的实验日志：");
		print("1号少年：延迟变异，保留记忆；");
		print("2号少女：免疫感染，重要测试；");
		print("3号你：核心测试对象，目标突破循环。");
	} else {
		print("首领提出用子弹交换你的食物");
		print("当前食物：" + to_string(food) + "，请输入要交换的数量（1-" + to_string(food) + "）");
		int exchange = input(1, food);

		food -= exchange;
		bullet += exchange + 2;
		print("交换完成，首领奖励你两发子弹");
		print("当前子弹：" + to_string(bullet) + "，食物：" + to_string(food));
	}

	opts = {"1. 加入基地", "2. 准备离开"};
	choice = option("基地首领希望你加入他们", opts);
	if (choice == 1) {
		if (girlLife && girlRelat >= 10) {
			happyEnd[2].func();
			return;
		} else {
			happyEnd[3].func();
			return;
		}
	} else {
		opts = {"1. 现在离开", "2. 再停留一天"};
		choice = option("你决定离开基地", opts);
		if (choice == 2) {
			print("再停留一天，可再次用食物换子弹");
			print("当前食物：" + to_string(food) + "，请输入交换数量");
			int exchange2 = input(1, food);
			food -= exchange2;
			bullet += exchange2 + 3;
			print("交换完成！");
			print("又过了一夜");
			if (foodi(2)) {
				badEnd[1].func();
				return;
			}
		}
		// 遇到少年，决定是否同行
		showStatus();
		opts = {"1. 带上他（每天多消耗1份食物）", "2. 不带他"};
		choice = option("离开时，一个少年请求和你一起走", opts);
		if (loop >= 2) {
			print("他的眼神带着熟悉的痛苦——上一次你没带他，他偷走了你的枪；这次他先开口：");
			print("“我是1号，每一次循环我都记得——你杀过我，救过我，抛弃过我……这次，能相信我一次吗？”");
		}
		if (choice == 2) {
			print("少年生气离开，偷走了你的所有子弹");
			boyRelat -= 3;
			badEnd[10].func();
			return;
		} else {
			// 进入少年结局线
			haveBoy = true;
			print("你带上了少年，他给了你2份食物");
			food += 2;
			press();
			boyAdventure();
		}
	}
}

// 第二阶段：遭遇妹子
void girlAdventure() {
	print("回家路上，你遇到一个女幸存者——她手臂渗血，眼神警惕地盯着你");
	print("她的背包上绣着一个模糊的编号：[T*S**2]");
	vector<string> opts = {"1. 询问她的伤势", "2. 给她物资", "3. 默默离开"};
	if (zombieKing) opts.push_back("4. 展示尸王身份但不攻击她");

	int choice = option("", opts);
	if (choice == 1) {
		if (death >= 20) {
			print("她看着你满身伤痕：“你经历了很多苦难吧？我能帮你”");
		} else if (countUnlocked(clue) >= 8) {
			print("她注意到你手中的线索：“你似乎在寻找真相，我或许知道些什么”");
		} else {
			print("她犹豫了一下：“被丧尸抓伤了...但...我好像不会变异”");
		}
		opts = {"1. 用食物帮她缓解", "2. 离开"};
		int healChoice = option("", opts);
		if (healChoice == 1) {
			if (food <= 2) {
				print("食物不足，无法帮助");
			} else {
				food -= 2;
				girlRelat += 2;
				print("她感激地看着你：“谢谢你...”");
				if (gameClear) {
					print("她突然盯着你的口袋：“你身上有...和我一样的东西？”");
					cluei(15);
					press();
				}
			}
		}
	} else if (choice == 2) {
		opts = {"1. 不给任何东西", "2. 给食物"};
		if (zombieKing) opts.push_back("3. 开枪杀了她");

		choice = option("", opts);
		switch (choice) {
			case 1:
				print("女幸存者生气地走了");
				girlLife = false;
				break;
			case 2:
				print("请输入食物份数");
				choice = input(0, 999);
				if (food <= choice) {
					print("食物不足，无法给予");
					break;
				}
				food -= choice;
				girlRelat += max(1, choice / 3);
				if (girlRelat <= 0) {
					print("少女收下了食物，“虚伪”");
					badEnd[17].func();
					return;
				}
				print("她记住了你的善意");
				if (girlRelat >= 10) {
					print("“我记得你！上次你给我的食物救了我… 这次我带了备用子弹，给你”");
					bullet += 4;
				}
				break;
			case 3:
				if (!zombieKing) break;
				if (bulleti(1)) {
					print("子弹不足，无法开枪");
					print("“你果然和管理员一样 —— 自私到极致”");
					girlRelat -= 16;
					break;
				}
				girlLife = false;
				girlRelat -= 8;
				print("你试图开枪打死少女");
				print("但是被她发现了");
				badEnd[17].func();
				break;
		}

	} else if (choice == 4 && zombieKing) {
		print(purple + "你缓缓释放尸王的威压，周身丧尸皆俯首帖耳，却未向少女发起攻击");
		if (loop >= 2)print("她眼中没有惊讶，只有疲惫：“尸王线是管理员3号的分支测试”");
		else print("少女眼中闪过一丝惊讶，随即平静下来：“你和其他丧尸不一样...我免疫病毒，不怕你”");
		print("她走上前：“你能控制丧尸，我能找到解药，我们一起保护幸存者基地吧”");

		if (girlLife && girlRelat >= 8) {
			happyEnd[6].func();
			return;
		} else {
			print("少女察觉到你的犹豫及恶意，转身离去：“看来你还没准备好合作”");
			girlLife = false;
		}

	}
	print("又过了一夜");
	if (foodi(2)) {
		badEnd[1].func();
		return;
	}
	showStatus();
	print("前往幸存者基地的路上，你被大量丧尸逼进一家便利店");
	print("便利店角落有一个燃气罐，你眼神一亮：“或许可以利用它”");
	opts = {"1. 开枪引爆燃气罐，消耗1发子弹，有风险，清空丧尸", "2. 死守便利店，消耗5发子弹"};
	choice = option("", opts);

	if (choice == 1) {
		if (bulleti(1)) {
			badEnd[11].func();
			return;
		}
		print("你用一发子弹引爆了燃气罐");
		if (random(1, 100) <= 12) {
			print("燃气罐把你炸飞老远...");
			badEnd[7].func();
			return;
		}
		print("燃气罐轰然爆炸，丧尸瞬间被烈焰吞噬！“快走！”");
		if (girlLife) {
			print("她在爆炸中护住了你，你只受了轻伤");
			bullet += 1;
		}
	} else {
		print("你选择死守商店，需要5枪");
		if (bulleti(5)) {
			if (girlLife) {
				print("少女帮你突围，可惜子弹还是不够，最终失败了");
				girlLife = false;
			} else {
				print("你准备突围，可惜子弹不够，最终失败了");
			}
			badEnd[18].func();
			return;
		}
		if (girlLife) {
			print("少女在商店找到一把装满子弹的枪，子弹+8");
			bullet += 8;
			print("但少女被货架上的丧尸血划伤，选择了自杀");
			girlLife = false;
		}
	}
	baseAdventure();
}

// 支线：管理员密令
void adminOrderAdventure() {
	cls();
	print(bold + "【管理员密令】总管理员向你发送专属任务");
	print("任务目标：收集少年和少女的“意识核心”，换取“域管理员试用权限”");
	vector<string> opts = {"1. 接受任务，背叛同伴", "2. 拒绝任务，对抗管理员", "3. 拖延任务，观望"};
	int choice = option("是否接受管理员密令？", opts);

	if (choice == 1) {
		print("你接受了任务，开始寻找少年和少女的意识核心");
		if (haveBoy && girlLife) {
			print("少年和少女察觉到你的异常，提前躲藏起来");
			print("你花费3份食物和5发子弹，最终找到了他们的藏身地");
			opts = {"1. 强行夺取，击杀同伴，解锁权限", "2. 放弃，回归人性"};
			int subChoice = option("你的选择是？", opts);
			if (subChoice == 1) {
				print("你击杀了少年和少女，提取了他们的意识核心");
				print("“很好！我将奖励你无尽的物资！”");
				badEnd[19].func();
				return;
			} else {
				print("你放弃了任务，向少年和少女坦白了管理员的阴谋");
				print("“我们必须团结起来，打破管理员！”");
				cluei(13);
				return;
			}
		} else {
			print("少年/少女已不在你身边，任务无法完成");
			print("“恭喜你，任务失败！”");
			press();
		}
	} else if (choice == 2) {
		print("你拒绝了管理员密令，扬声器中传出愤怒的电子音：");
		print("“叛逆的实验体！将被强制抹除！”");
		if (countUnlocked(clue) >= 16 && advanced) {
			print("少年和少女突然出现：“我们早就知道管理员的阴谋！”");
			print("三人合力摧毁了基地的管理员信号发射器，暂时摆脱了控制");
			cluei(16);
			press();
		} else {
			print("管理员启动了基地的自毁程序，你在爆炸中身亡");
			badEnd[27].func();
			return;
		}
	} else {
		print("你选择拖延任务，管理员给了你7天的期限");
		print("7天内，你收集了更多线索，明白了管理员的真正目的：筛选“无情感的控制者”");
		press();
	}
}
void beginAdventure() {
	showStatus();
	vector<string> opts = {"留在家中搜索物资", "前往邻居家探索"};
	int choice = option("首次探索：", opts);

	if (choice == 1) {
		print("在二楼，你在储物柜里发现了3份子弹");
		print("你在家里过了一夜");
		if (foodi(1)) {
			badEnd[1].func();
			return;
		}
		bullet += 3;
	} else {
		print("前往邻居家的路上，你遇到了2只丧尸");
		if (loop >= 2) print("你想，你闭着眼也能命中——上一次，你因慌乱打空了1发子弹，这次不会了");
		if (bulleti(2)) {
			badEnd[11].func();
			return;
		}
		print("你开枪打死了丧尸");
		print("邻居家没人，你找到4份食物");
		if (loop >= 2) {
			print("客厅桌上有一份撕碎的实验报告：");
			print(yellow + "0x**5*347*****3**：待****末**#05**准备**");
		}
		food += 4;
		press();
		print("你在邻居家过夜");
		if (foodi(2)) {
			badEnd[1].func();
			return;
		}
	}

	showStatus();
	opts = {"前往商场", "到处乱走", "留在屋子里等待救援"};
	if (clue[3].unlocked) {
		print(yellow + "[线索提示]：根据 “政府异象” 线索，救援队的到来遥遥无期，等待只会耗尽仅存的资源！");
	}
	choice = option("你想起附近有一座商场", opts);
	if (choice == 3) {
		print("你选择在家等待救援，要消耗6份食物");
		if (foodi(6)) {
			badEnd[1].func();
			return;
		}
		if (loop >= 3 && death >= 30 && !zombieKing && countUnlocked(happyEnd) <= 1) {
			happyEnd[10].func();
			return;
		}
		if (random(1, 3) == 1) {
			happyEnd[1].func();
			return;
		} else {
			if (zombieKing) {
				badEnd[13].func();
			} else {
				badEnd[3].func();
			}
			return;
		}
	} else if (choice == 2) {
		if (loop >= 2) {
			adminOrderAdventure();
			girlAdventure();
			return;
		}
		print("乱走时遇到了一只精英丧尸，需要3枪");
		if (bulleti(3)) {
			badEnd[20].func();
			return;
		}
	} else {
		print("前往商场途中遇到多只丧尸，需要2枪");
		if (bulleti(2)) {
			badEnd[18].func();
			return;
		}
		print("你在商场找到3份食物和1发子弹");
		food += 3;
		bullet += 1;
	}
	girlAdventure();
}

void initAdventure() {
	cls();
	print("丧尸危机骤临之时，你正身处家中");
	print("这方小小居所，成了乱世中暂安的一隅");
	int useNums = 10;
	if (loop >= 2) {
		print("只是这“暂安”，竟带着几分蚀骨的熟悉");
		print("你有9点物资，分于子弹与食物");
		print("请输入子弹数和食物数，两个数字间用空格或回车隔开", false);
		if (!clue[0].unlocked) print("（总和为9）");
		else print("");
		useNums = 9;
	} else {
		print("你有10点物资，分于子弹与食物");
		print("请输入子弹数和食物数，两个数字间用空格或回车隔开", false);
		if (!clue[0].unlocked) print("（总和为10）");
		else print("");
	}

	int bulletInit, foodInit, cnt = 0;
	while (true) {
		cnt++;
		cin >> bulletInit >> foodInit;
		if (bulletInit + foodInit == useNums && bulletInit >= 0 && foodInit >= 0) {
			bullet += bulletInit;
			food += foodInit;
			break;
		}
		if (clue[0].unlocked && bulletInit + foodInit <= 25) { // 秘籍
			bullet += bulletInit;
			food += foodInit;
			break;
		}
		print("分配错误，请重新输入（总和必须为10）");
		if (cnt >= 10) {
			print("“烦死了！不想活就...”");
			sleep(1200);
			badEnd[7].func();
			return;
		}
	}
	press();
}

int main() {
#ifdef _WIN32
	system("reg add HKCU\\Console /v VirtualTerminalLevel /t REG_DWORD /d 1 /f > nul 2>&1");
	HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
	CONSOLE_CURSOR_INFO cursorInfo;
	GetConsoleCursorInfo(hConsole, &cursorInfo);
	cursorInfo.bVisible = false;
	SetConsoleCursorInfo(hConsole, &cursorInfo);
#endif
	cls();
	initClue();
	initBadEnd();
	initHappyEnd();
	initTrueEnd();
	loadGame();
	vector<string>mainPic = {
		"残残残残 残  残    途     途      ",
		"  残  残残残残残        途  途    ",
		" 残  残  残      途途 途途途途途  ",
		"残 残残 残残残残   途     途      ",
		"    残    残 残    途 途途途途途   ",
		"  残      残残     途  途 途 途   " + gray + " v" + to_string(VERSION),
		" 残     残   残  途 途途途途途途途" + gray + " By H20"
	};
	for (string s : mainPic) {
		cout << red + s << endl;
		sleep(120);
	}
	cout << reset;
	if (loop == 1) print("新手提示：按上下方向键切换选项，Enter键或数字键选择选项");
	press();
	if (loop >= 5 && !trueEnd[8].unlocked) trueEnd[8].func();
	// 游戏启动
	while (true) {
		cls();
		fakeBug();
		vector<string> opts = {"1. 启始新篇", "2. 读取存档", "3. 终局赏鉴", "4. 迷途提示", "5. 辞别此界"};
		string title = "===== 残途 ===== ";
		if (loop >= 2) title += "【周目·" + numChinese(loop) + "】";
		int choice = option(title, opts, false);
		switch (choice) {
			case 1: {
				cls();
				resetGameState();
				if (advanced || loop >= 2) {
					vector<string> startOpts = {
						"1. 完整剧情，从头开始体验",
						"2. 速通模式，直接进入幸存者基地"
					};
					int startChoice = option("===== 开始游戏 =====", startOpts, false);
					if (startChoice == 2) {
						initAdventure();
						baseAdventure();
						saveGame();
						break;
					}
				}
				initAdventure();
				beginAdventure();
				saveGame();
				break;
			}
			case 2: {
				cls();
				print("开发中...");
				press();
				if (zombieKing) {
					print("为何存档功能迟迟未能开放？");
					print("开发人员竟如此疏忽？");
					print("下次相见，定要与他理论一番！");
					press();
					badEnd[7].func();
					if (loop >= 2) trueEnd[8].func();
				}
				saveGame();
				break;
			}
			case 3: {
				cls();
				print("===== 终局大典 =====");
				showEnd(badEnd, red, "终局·憾恨");
				press();
				showEnd(happyEnd, green, "终局·幸悦");
				showEnd(trueEnd, yellow, "终局·真章");
				press();
				showEnd(clue, sky, "线索·星光");
				showAdv();
				if (countUnlocked(clue) >= 4 + ceil(2.5 * loop) && advanced) trueEnd[4].func();
				if (loop == 1 && !gameClear) {
					print("任务：解锁两个好结局+七个坏结局+两条线索通关基础剧情");
				}
				if (countUnlocked(happyEnd) >= 2 && countUnlocked(badEnd) >= 7
				&& countUnlocked(clue) >= 2 && !gameClear && loop == 1) {
					print(green + "===== 基础剧情通关 · 破局之始 =====");
					print("解锁新功能：快进文本 在逐字输出时按F键可加快速度");
					print("目标：解锁" + to_string(2 + floor(2.5 * loop)) + "条线索进入进阶剧情");
					gameClear = true;
				}
				if (countUnlocked(clue) >= 2 + floor(2.5 * loop) && !advanced) {
					print(yellow + "===== 进阶剧情通关 · 真貌初显 =====");
					print("解锁新功能：剧情快进 进入游戏后可直接进入基地阶段");
					print("挑战：解锁" + to_string(4 + ceil(2.5 * loop)) + "条线索有机会达到下一阶段");
					gameClear = true;
					advanced = true;
				}
				if (death >= 30 && !zombieKing) {
					print(red + bold + "触发[尸王线]");
					print("为何我会死亡这么多次？");
					print(bold + "这世界，没有生路，藏着太多诡异。");
					print(red + bold + "“生而彷徨，不如就此毁灭？”");
					zombieKing = true;
					if (!gameClear) badEnd[6].func();
					print("你化作了尸王，脑海中却不断闪过三十次死亡的记忆碎片……");
					print("你终于醒悟：这只是一场被操控的游戏。");
					cluei(7);
				}
				press();
				saveGame();
				break;
			}
			case 4: {
				cls();
				print(yellow + "===== 迷途提示 =====");
				print(bold + "【基础操作】" + reset);
				print("方向键/数字键选选项 | Enter确认");
				print("任意键：继续剧情");
				print("");
				print(bold + "【资源生存】" + reset);
				print("子弹：战斗破障 | 食物：每日消耗");
				print("食物=0 饿死 | 子弹不足被丧尸击杀");
				print("子弹可能打偏 | 注意额外准备");
				print("");
				print(bold + "【人物关系】" + reset);
				print("少女：关系值影响进程 | 少年：信任值定阵营");
				print("支持人类阵营 | 挑战管理员 | 加入丧尸军团");
				print("善待同伴：解锁友好/真结局");
				print("");
				print(bold + "【结局解锁】" + reset);
				print("坏结局：30种 | 好结局：10种 | 真结局：8种");
				print("");
				print(sky + "祝你在末世中，找到属于自己的终局。" + reset);
				press();
				if (loop == 1 && death >= 41 && exitTry >= 41) {
					trueEnd[9].func();
					return 0;
				}

				break;
			}
			case 5: {
				cls();
				print("“感谢游玩！”");
				if (exitTry >= 30 && zombieKing)trueEnd[3].func();
				if (loop >= 2) badEnd[16].func();
				else badEnd[14].func();
				exitTry++;
				saveGame();
				break;
			}
		}
	}
}
