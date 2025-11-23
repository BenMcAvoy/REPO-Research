#pragma once

#include <mono/all.h>
#include <print>
#include <span>
#include <string_view>
#include <format>

// Global variables
extern HMODULE gHMono;
extern MonoImage* gAssemblyCSharp;
extern MonoImage* gUnityEngineCore;

// Mono function pointers
struct MP {
	static inline decltype(&mono_get_root_domain) mono_get_root_domain = nullptr;
	static inline decltype(&mono_thread_attach) mono_thread_attach = nullptr;
	static inline decltype(&mono_domain_assembly_open) mono_domain_assembly_open = nullptr;
	static inline decltype(&mono_assembly_get_image) mono_assembly_get_image = nullptr;
	static inline decltype(&mono_class_from_name) mono_class_from_name = nullptr;
	static inline decltype(&mono_class_get_method_from_name) mono_class_get_method_from_name = nullptr;
	static inline decltype(&mono_runtime_invoke) mono_runtime_invoke = nullptr;
	static inline decltype(&mono_class_get_field_from_name) mono_class_get_field_from_name = nullptr;
	static inline decltype(&mono_field_get_value) mono_field_get_value = nullptr;
	static inline decltype(&mono_field_set_value) mono_field_set_value = nullptr;
	static inline decltype(&mono_class_get_name) mono_class_get_name = nullptr;
	static inline decltype(&mono_class_get_namespace) mono_class_get_namespace = nullptr;
	static inline decltype(&mono_object_get_class) mono_object_get_class = nullptr;
	static inline decltype(&mono_class_get_methods) mono_class_get_methods = nullptr;
	static inline decltype(&mono_class_get_fields) mono_class_get_fields = nullptr;
	static inline decltype(&mono_assembly_foreach) mono_assembly_foreach = nullptr;
	static inline decltype(&mono_image_get_name) mono_image_get_name = nullptr;
	static inline decltype(&mono_image_get_table_info) mono_image_get_table_info = nullptr;
	static inline decltype(&mono_table_info_get_rows) mono_table_info_get_rows = nullptr;
	static inline decltype(&mono_metadata_decode_row) mono_metadata_decode_row = nullptr;
	static inline decltype(&mono_metadata_string_heap) mono_metadata_string_heap = nullptr;
	static inline decltype(&mono_method_get_name) mono_method_get_name = nullptr;
	static inline decltype(&mono_field_static_get_value) mono_field_static_get_value = nullptr;
	static inline decltype(&mono_class_vtable) mono_class_vtable = nullptr;
	static inline decltype(&mono_field_get_offset) mono_field_get_offset = nullptr;
	static inline decltype(&mono_array_length) mono_array_length = nullptr;
	static inline decltype(&mono_array_addr_with_size) mono_array_addr_with_size = nullptr;
	static inline decltype(&mono_class_array_element_size) mono_class_array_element_size = nullptr;
	static inline decltype(&mono_field_get_name) mono_field_get_name = nullptr;
	static inline decltype(&mono_string_to_utf8) mono_string_to_utf8 = nullptr;
	static inline decltype(&mono_object_unbox) mono_object_unbox = nullptr;
	static inline decltype(&mono_compile_method) mono_compile_method = nullptr;
	static inline decltype(&mono_value_box) mono_value_box = nullptr;
	static inline decltype(&mono_lookup_internal_call) mono_lookup_internal_call = nullptr;
	static inline decltype(&mono_gchandle_new) mono_gchandle_new = nullptr;
	static inline decltype(&mono_gchandle_free) mono_gchandle_free = nullptr;
	static inline decltype(&mono_gchandle_get_target) mono_gchandle_get_target = nullptr;
};

// Unity types
enum MonoOrStereoscopicEye {
    Left = 0,
    Right = 1,
    Mono = 2,
};

struct Vector3 {
	float x;
	float y;
	float z;
};

struct Matrix4x4 {
	float m[16]{};

	inline float get(int row, int col) const { return m[col * 4 + row]; } // column-major accessor
	inline void set(int row, int col, float v) { m[col * 4 + row] = v; }

	// Row-major accessor (transposed view) for autodetection/comparison.
	inline float getRowMajor(int row, int col) const { return m[row * 4 + col]; }

	Matrix4x4 operator*(const Matrix4x4& other) const {
		Matrix4x4 result;
		for (int col = 0; col < 4; ++col) {
			for (int row = 0; row < 4; ++row) {
				float sum = 0.0f;
				for (int k = 0; k < 4; ++k) {
					sum += get(row, k) * other.get(k, col);
				}
				result.set(row, col, sum);
			}
		}
		return result;
	}

	Matrix4x4& operator*=(const Matrix4x4& other) { *this = *this * other; return *this; }
};

struct Vector4 {
	float x, y, z, w;
};

#define RESOLVE(name) do { \
	MP::name = reinterpret_cast<decltype(MP::name)>(GetProcAddress(gHMono, #name)); \
	if (!MP::name) { \
		MessageBoxA(NULL, std::format("Failed to resolve {}", #name).c_str(), "ERROR", MB_OK); \
		return 1; \
	} \
} while (0)

// Initialize all Mono function pointers
inline int InitializeMonoFunctions() {
	RESOLVE(mono_get_root_domain);
	RESOLVE(mono_thread_attach);
	RESOLVE(mono_domain_assembly_open);
	RESOLVE(mono_assembly_get_image);
	RESOLVE(mono_class_from_name);
	RESOLVE(mono_class_get_method_from_name);
	RESOLVE(mono_runtime_invoke);
	RESOLVE(mono_class_get_field_from_name);
	RESOLVE(mono_field_get_value);
	RESOLVE(mono_field_set_value);
	RESOLVE(mono_class_get_name);
	RESOLVE(mono_class_get_namespace);
	RESOLVE(mono_object_get_class);
	RESOLVE(mono_class_get_methods);
	RESOLVE(mono_class_get_fields);
	RESOLVE(mono_assembly_foreach);
	RESOLVE(mono_image_get_name);
	RESOLVE(mono_image_get_table_info);
	RESOLVE(mono_table_info_get_rows);
	RESOLVE(mono_metadata_decode_row);
	RESOLVE(mono_metadata_string_heap);
	RESOLVE(mono_method_get_name);
	RESOLVE(mono_field_static_get_value);
	RESOLVE(mono_class_vtable);
	RESOLVE(mono_field_get_offset);
	RESOLVE(mono_array_length);
	RESOLVE(mono_array_addr_with_size);
	RESOLVE(mono_class_array_element_size);
	RESOLVE(mono_field_get_name);
	RESOLVE(mono_string_to_utf8);
	RESOLVE(mono_object_unbox);
	RESOLVE(mono_compile_method);
	RESOLVE(mono_value_box);
	RESOLVE(mono_lookup_internal_call);
	RESOLVE(mono_gchandle_new);
	RESOLVE(mono_gchandle_free);
	RESOLVE(mono_gchandle_get_target);
	return 0;
}

// Assembly callback
inline void Callback(MonoAssembly* assembly, void* ud) {
    MonoImage* img = MP::mono_assembly_get_image(assembly);
	if (!img) {
		MessageBoxA(NULL, "Failed to get image!", "ERROR", MB_OK);
		return;
	}

    const char* name = MP::mono_image_get_name(img);
	std::println("Assembly at {:p} w/ image name: \"{}\"", (void*)assembly, name);

	std::string nameStr(name);
	if (strcmp(name, "Assembly-CSharp") == 0) {
		std::println("\n\nFound Assembly-CSharp.dll at {:p}!\n\n", (void*)assembly);
		*(static_cast<MonoImage**>(ud)) = img;
	}

	// UnityEngine.CoreModule
	if (strcmp(name, "UnityEngine.CoreModule") == 0) {
		std::println("\n\nFound UnityEngine.CoreModule.dll at {:p}!\n\n", (void*)assembly);
		gUnityEngineCore = img;
	}
}

// Dump types
inline void DumpTypes(MonoImage* img) {
	const MonoTableInfo* table = MP::mono_image_get_table_info(img, MONO_TABLE_TYPEDEF);
	int rows = MP::mono_table_info_get_rows(table);

	for (int i = 0; i < rows; i++) {
		uint32_t cols[MONO_TYPEDEF_SIZE];
		MP::mono_metadata_decode_row(table, i, cols, MONO_TYPEDEF_SIZE);

		const char* ns = MP::mono_metadata_string_heap(img, cols[MONO_TYPEDEF_NAMESPACE]);
		const char* name = MP::mono_metadata_string_heap(img, cols[MONO_TYPEDEF_NAME]);

		if (!name || name[0] == '\0') continue;

		if (ns[0] == '\0')
			std::println("TypeDef[{}]: {}", i + 1, name);
		else
			std::println("TypeDef[{}]: {}.{}", i + 1, ns, name);
	}
}

// Get field from MonoObject
template<typename T>
T GetField(MonoObject* obj, const char* fieldName) {
	MonoClass* klass = MP::mono_object_get_class(obj);
	if (!klass) return T{}; // or nullptr for pointer types

	MonoClassField* field = MP::mono_class_get_field_from_name(klass, fieldName);
	if (!field) return T{};

	if constexpr (std::is_pointer_v<T>) {
		MonoObject* value = nullptr;
		MP::mono_field_get_value(obj, field, &value);
		return reinterpret_cast<T>(value);
	}
	else {
		T value{};
		MP::mono_field_get_value(obj, field, &value);
		return value;
	}
}

template <typename T> // T = MonoObject* or primitive unboxed type
void SetField(MonoObject* obj, const char* fieldName, T value) {
	MonoClass* klass = MP::mono_object_get_class(obj);
	if (!klass) return;

	MonoClassField* field = MP::mono_class_get_field_from_name(klass, fieldName);
	if (!field) return;

	if constexpr (std::is_pointer_v<T>) {
		MonoObject* valObj = reinterpret_cast<MonoObject*>(value);
		MP::mono_field_set_value(obj, field, &valObj);
	}
	else {
		MP::mono_field_set_value(obj, field, &value);
	}
}

// Get static field
inline MonoObject* GetStaticField(const char* className, const char* fieldName) {
	MonoClass* klass = MP::mono_class_from_name(gAssemblyCSharp, "", className);
	if (klass == nullptr) {
		std::println("Failed to get class '{}' from Assembly-CSharp.dll", className);
		return nullptr;
	}

	MonoClassField* field = MP::mono_class_get_field_from_name(klass, fieldName);
	if (field == nullptr) {
		std::println("Failed to get static field '{}' for class at {:p}", fieldName, (void*)klass);
		return nullptr;
	}

	MonoVTable* vt = MP::mono_class_vtable(MP::mono_get_root_domain(), klass);
	if (vt == nullptr) {
		std::println("Failed to get vtable for class at {:p}", (void*)klass);
		return nullptr;
	}

	MonoObject* value = nullptr;
	MP::mono_field_static_get_value(vt, field, &value);

	if (value == nullptr) {
		std::println("Static field '{}' is null on class at {:p}", fieldName, (void*)klass);
		return nullptr;
	}

	return value;
}

// Print fields
inline void PrintFields(MonoObject* obj) {
	MonoClass* klass = MP::mono_object_get_class(obj);

	const char* className = MP::mono_class_get_name(klass);
	const char* classNamespace = MP::mono_class_get_namespace(klass);
	std::println("Class: {}{}{}", classNamespace, classNamespace[0] != '\0' ? "." : "", className);

	MonoClassField* fieldIter = nullptr;
	void* iter = nullptr;
	while ((fieldIter = MP::mono_class_get_fields(klass, &iter)) != nullptr) {
		const char* fieldName = MP::mono_field_get_name(fieldIter);
		std::println("- Field: {}", fieldName);
	}
}

// Read List<T>
inline std::span<MonoObject*> ReadList(MonoObject* obj) {
	MonoClass* listClass = MP::mono_object_get_class(obj);

	MonoClassField* itemsField = MP::mono_class_get_field_from_name(listClass, "_items");
	MonoClassField* sizeField = MP::mono_class_get_field_from_name(listClass, "_size");

	int itemsOffset = MP::mono_field_get_offset(itemsField);
	int sizeOffset = MP::mono_field_get_offset(sizeField);

	MonoArray* data = *(MonoArray**)(reinterpret_cast<uint8_t*>(obj) + itemsOffset);
	int size = *(int*)(reinterpret_cast<uint8_t*>(obj) + sizeOffset);
	std::span<MonoObject*> span((MonoObject**)(MP::mono_array_addr_with_size(data, sizeof(MonoObject*), 0)), size);
	return span;
}

// Read string
inline std::string_view ReadString(MonoObject* obj) {
	MonoString* monoStr = reinterpret_cast<MonoString*>(obj);
	char* cStr = MP::mono_string_to_utf8(monoStr);
	return std::string_view(cStr);
}

// Get method
inline MonoMethod* GetMethod(std::string_view nsName, std::string_view className, std::string_view methodName, int paramCount, MonoImage* image = gAssemblyCSharp) {
	MonoClass* klass = MP::mono_class_from_name(image, nsName.data(), className.data());
	if (klass == nullptr) {
		std::println("Failed to get class '{}.{}' from Assembly-CSharp.dll", nsName, className);
		return nullptr;
	}

	MonoMethod* method = MP::mono_class_get_method_from_name(klass, methodName.data(), paramCount);
	if (method == nullptr) {
		std::println("Failed to get method '{}' for class at {:p}", methodName, (void*)klass);
		return nullptr;
	}

	return method;
}

// World to screen conversion using manual matrix math (games function doesn't line up properly)
inline bool WorldToScreen(Vector3 worldPos, ImVec2& out) {
	static MonoMethod* getMainCameraMethod =
		GetMethod("UnityEngine", "Camera", "get_main", 0, gUnityEngineCore);

	MonoObject* exception = nullptr;
	MonoObject* cameraObj = MP::mono_runtime_invoke(getMainCameraMethod, nullptr, nullptr, &exception);

	if (exception != nullptr || cameraObj == nullptr) {
		return false;
	}

	static MonoMethod* getWorldToCameraMatrixMethod =
		GetMethod("UnityEngine", "Camera", "get_worldToCameraMatrix_Injected", 1, gUnityEngineCore);
	static MonoMethod* getProjectionMatrixMethod =
		GetMethod("UnityEngine", "Camera", "get_projectionMatrix_Injected", 1, gUnityEngineCore);

	Matrix4x4 worldToCameraMatrix = {};
	void* args[] = { &worldToCameraMatrix };

	MP::mono_runtime_invoke(getWorldToCameraMatrixMethod, cameraObj, args, &exception);
	if (exception != nullptr)
		return false;

	Matrix4x4 projectionMatrix = {};
	args[0] = &projectionMatrix;

	MP::mono_runtime_invoke(getProjectionMatrixMethod, cameraObj, args, &exception);
	if (exception != nullptr)
		return false;

	Matrix4x4 vpMatrix = projectionMatrix * worldToCameraMatrix;

	// Unity uses column-major matrices
	Vector4 clipSpacePos{};
	clipSpacePos.x = worldPos.x * vpMatrix.get(0,0) + worldPos.y * vpMatrix.get(0,1) + worldPos.z * vpMatrix.get(0,2) + vpMatrix.get(0,3);
	clipSpacePos.y = worldPos.x * vpMatrix.get(1,0) + worldPos.y * vpMatrix.get(1,1) + worldPos.z * vpMatrix.get(1,2) + vpMatrix.get(1,3);
	clipSpacePos.z = worldPos.x * vpMatrix.get(2,0) + worldPos.y * vpMatrix.get(2,1) + worldPos.z * vpMatrix.get(2,2) + vpMatrix.get(2,3);
	clipSpacePos.w = worldPos.x * vpMatrix.get(3,0) + worldPos.y * vpMatrix.get(3,1) + worldPos.z * vpMatrix.get(3,2) + vpMatrix.get(3,3);

	if (clipSpacePos.w == 0.0f) return false;

	if (clipSpacePos.w < 0.1f)
		return false;

	ImGuiIO& io = ImGui::GetIO();
	out.x = (io.DisplaySize.x / 2.0f) * (clipSpacePos.x / clipSpacePos.w + 1.0f);
	out.y = (io.DisplaySize.y / 2.0f) * (1.0f - clipSpacePos.y / clipSpacePos.w);

	return true;
}
