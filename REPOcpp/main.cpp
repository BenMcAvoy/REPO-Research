#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

#include <print>
#include <span>
#include <format>

#include <imgui.h>
#include <dxgi.h>

#include "present.h"
#include "monoHelpers.h"

HMODULE gHMono = NULL;
MonoImage* gAssemblyCSharp = nullptr;
MonoImage* gUnityEngineCore = nullptr;

bool IsDestroyed(MonoObject* obj) {
    static MonoMethod* opEq = GetMethod("UnityEngine", "Object", "op_Equality", 2, gUnityEngineCore);
    void* args[] = { obj, nullptr };
    MonoObject* result = MP::mono_runtime_invoke(opEq, nullptr, args, nullptr);
    return *(bool*)MP::mono_object_unbox(result);
}

static void DrawLabelCentered(const std::string& text, ImVec2 center, ImU32 bg, ImU32 fg) {
	ImDrawList* dl = ImGui::GetBackgroundDrawList();
	ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
	ImVec2 boxSize(textSize.x + 8.0f, textSize.y + 4.0f);
	ImVec2 pos = center;
	ImVec2 bbMin(pos.x - boxSize.x * 0.5f, pos.y - boxSize.y * 0.5f);
	ImVec2 bbMax(pos.x + boxSize.x * 0.5f, pos.y + boxSize.y * 0.5f);
	const float rounding = 3.0f;

	dl->AddRectFilled(bbMin, bbMax, bg, rounding);
	dl->AddRect(bbMin, bbMax, IM_COL32(0, 0, 0, 90), rounding, 0, 1.0f);

	ImVec2 textPos(bbMin.x + 4.0f, bbMin.y + 2.0f);
	dl->AddText(ImVec2(textPos.x + 1, textPos.y + 1), IM_COL32(0, 0, 0, 200), text.c_str());
	dl->AddText(textPos, fg, text.c_str());
}

static void DrawValuableESP() {
	ImVec2 screenPos;

	MonoObject* oValuableDirector = GetStaticField("ValuableDirector", "instance");
	auto oValuableList = GetField<MonoObject*>(oValuableDirector, "valuableList");
	std::span<MonoObject*> valuablesSpan = ReadList(oValuableList);
	for (MonoObject* oValuableObject : valuablesSpan) {
		if (!oValuableObject) continue;

		auto oDollarValueCurrent = GetField<float>(oValuableObject, "dollarValueCurrent");
		auto oPhysGrab = GetField<MonoObject*>(oValuableObject, "physGrabObject");
		auto centerPoint = GetField<Vector3>(oPhysGrab, "centerPoint");

		SetField<bool>(oValuableObject, "discovered", true);
		SetField<bool>(oValuableObject, "discoveredReminder", true);

		MonoMethod* getGameObjectMethod = GetMethod("UnityEngine", "Component", "get_gameObject", 0, gUnityEngineCore);
		MonoObject* exception = nullptr;
		MonoObject* oGameObject = MP::mono_runtime_invoke(getGameObjectMethod, oPhysGrab, nullptr, &exception);
		if (exception != nullptr || !oGameObject)
			continue;

		if (IsDestroyed(oGameObject))
			continue;

		MonoMethod* getNameMethod = GetMethod("UnityEngine", "Object", "get_name", 0, gUnityEngineCore);
		MonoObject* oNameObj = MP::mono_runtime_invoke(getNameMethod, oGameObject, nullptr, &exception);
		if (exception != nullptr || !oNameObj)
			continue;

		auto name = ReadString(oNameObj);
		std::string nameStr(name);

		size_t clonePos = nameStr.find("(Clone)");
		if (clonePos != std::string::npos) {
			nameStr = nameStr.substr(0, clonePos);
		}

		size_t pos = 0;
		int currentWord = 0;
		while (currentWord < 2 && pos != std::string::npos) {
			pos = nameStr.find(' ', pos);
			if (pos != std::string::npos) {
				currentWord++;
				pos++;
			}
		}

		if (pos != std::string::npos && pos < nameStr.size()) {
			nameStr = nameStr.substr(pos);
		} else {
			nameStr.clear();
		}

		if (WorldToScreen(centerPoint, screenPos)) {
			auto text = std::format("${} - {}", static_cast<int>(oDollarValueCurrent), nameStr);
			DrawLabelCentered(text, screenPos, IM_COL32(141, 117, 85, 150), IM_COL32(255, 215, 0, 255));
		}
	}
}

static void DrawItemESP() {
	ImVec2 screenPos;

	MonoObject* oItemManager = GetStaticField("ItemManager", "instance");
	auto oItemList = GetField<MonoObject*>(oItemManager, "spawnedItems");
	std::span<MonoObject*> itemSpan = ReadList(oItemList);
	for (MonoObject* oItemAttributes : itemSpan) {
		if (!oItemAttributes) continue;

		auto oItemName = GetField<MonoObject*>(oItemAttributes, "itemName");
		auto name = ReadString(oItemName);

		auto oPhysGrab = GetField<MonoObject*>(oItemAttributes, "physGrabObject");
		auto centerPoint = GetField<Vector3>(oPhysGrab, "centerPoint");

		if (WorldToScreen(centerPoint, screenPos)) {
			std::string text = std::format("{}", name);
			DrawLabelCentered(text, screenPos, IM_COL32(52, 151, 52, 150), IM_COL32(52, 251, 52, 255));
		}
	}
}

static void DrawEnemyESP() {
	ImVec2 screenPos;

	MonoObject* oEnemyDirector = GetStaticField("EnemyDirector", "instance");
	auto oEnemyList = GetField<MonoObject*>(oEnemyDirector, "enemiesSpawned");
	std::span<MonoObject*> enemySpan = ReadList(oEnemyList);
	for (MonoObject* oEnemyParent : enemySpan) {
		if (!oEnemyParent) continue;

		auto oEnemy = GetField<MonoObject*>(oEnemyParent, "Enemy");
		if (!oEnemy) continue;

		auto oCenterTransform = GetField<MonoObject*>(oEnemy, "CenterTransform");

		static MonoMethod* getPositionMethod =
			GetMethod("UnityEngine", "Transform", "get_position_Injected", 1, gUnityEngineCore);
		Vector3 oPosition = {};
		void* args[] = { &oPosition };
		MonoObject* exception = nullptr;
		MP::mono_runtime_invoke(getPositionMethod, oCenterTransform, args, &exception);
		if (exception != nullptr)
			continue;

		if (WorldToScreen(oPosition, screenPos)) {
			auto oEnemyName = GetField<MonoObject*>(oEnemyParent, "enemyName");
			auto name = ReadString(oEnemyName);

			auto oHasDistance = GetField<bool>(oEnemy, "HasPlayerDistance");
			float oDistance = 0.0f;
			if (oHasDistance) {
				auto oDistanceComponent = GetField<MonoObject*>(oEnemy, "PlayerDistance");
				oDistance = GetField<float>(oDistanceComponent, "PlayerDistanceLocal");
			}

			auto oHasOnScreen = GetField<bool>(oEnemy, "HasOnScreen");
			bool oIsOnScreen = false;
			if (oHasOnScreen) {
				auto oEnemyOnScreenComponent = GetField<MonoObject*>(oEnemy, "OnScreen");
				oIsOnScreen = GetField<bool>(oEnemyOnScreenComponent, "OnScreenLocal");
			}

			if (oDistance <= 0.0f)
				continue;

			std::string text = std::format("{} ({:.1f}m){}", name, oDistance, oIsOnScreen ? " [VIS]" : "");
			DrawLabelCentered(text, screenPos, IM_COL32(151, 52, 52, 150), IM_COL32(251, 52, 52, 255));
		}
	}
}

static void DrawPlayerESP() {
	ImVec2 screenPos;

	MonoObject* oGameDirector = GetStaticField("GameDirector", "instance");
	auto oPlayerList = GetField<MonoObject*>(oGameDirector, "PlayerList");
	std::span<MonoObject*> playerSpan = ReadList(oPlayerList);
	for (MonoObject* oPlayerAvatar : playerSpan) {
		if (!oPlayerAvatar) continue;

		auto clientPosition = GetField<Vector3>(oPlayerAvatar, "clientPosition");
		auto oPlayerDeathHead = GetField<MonoObject*>(oPlayerAvatar, "playerDeathHead");

		if (WorldToScreen(clientPosition, screenPos)) {
			auto oPlayerName = GetField<MonoObject*>(oPlayerAvatar, "playerName");
			auto name = ReadString(oPlayerName);
			std::string text = std::format("{}", name, (void*)oPlayerDeathHead);
			DrawLabelCentered(text, screenPos, IM_COL32(52, 101, 151, 150), IM_COL32(100, 201, 251, 255));
		}
	}
}

DWORD MainThread(LPVOID lpParam) {
	AllocConsole();
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);

	gHMono = LoadLibraryA("mono-2.0-bdwgc.dll");
	if (!gHMono) return 1;

	if (InitializeMonoFunctions() != 0) return 1;

	MonoDomain* domain = MP::mono_get_root_domain();
	MP::mono_thread_attach(domain); // We can now call Mono functions on this thread! yay!

	MonoImage* asmCSharp = nullptr;
	MP::mono_assembly_foreach(reinterpret_cast<MonoFunc>(Callback), static_cast<void*>(&asmCSharp));
	gAssemblyCSharp = asmCSharp;

	//DumpTypes(asmCSharp);

	static bool* outerTakesFocus = nullptr;
    auto& presentManager = PresentManager_t::get();
	presentManager.setOnPresentCallback([](bool& takesFocus, IDXGISwapChain* /*pSwapChain*/, UINT /*SyncInterval*/, UINT /*Flags*/) {
		outerTakesFocus = &takesFocus;

		static bool attached = false;
		if (!attached) {
			MP::mono_thread_attach(MP::mono_get_root_domain());
			attached = true;
		}

		DrawValuableESP();
		DrawItemESP();
		DrawEnemyESP();
		DrawPlayerESP();
		});

    while (!GetAsyncKeyState(VK_END)) {
        if (GetAsyncKeyState(VK_INSERT) & 1) {
            if (outerTakesFocus) *outerTakesFocus = !*outerTakesFocus;
        }

        Sleep(100);
    }

	presentManager.shutdown();
	Sleep(500); // Give some time for any in-flight presents to finish

	FreeLibraryAndExitThread(static_cast<HMODULE>(lpParam), 0);
	return 0;
}

BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
	UNREFERENCED_PARAMETER(lpvReserved);

	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDLL);
		CreateThread(0, 0, MainThread, hinstDLL, 0, 0);
	}

	return TRUE;
}