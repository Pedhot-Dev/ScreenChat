#ifndef LOADER_H
#define LOADER_H

#if __has_include(<d3d9.h>)
#include <d3d9.h>
#endif
#if __has_include(<ProxyDX9/ProxyDX9.h>)
#include <ProxyDX9/ProxyDX9.h>
#endif
#if __has_include(<DrawHook/DrawHook.h>)
#include <DrawHook/DrawHook.h>
#endif
#if __has_include(<SRCursor/SRCursor.h>)
#include <SRCursor/SRCursor.h>
#endif
#if __has_include(<SREvents/SREvents.h>)
#include <SREvents/SREvents.h>
#endif

extern struct stGClass {
#if __has_include(<d3d9.h>)
	/// Указатель на объект IDirect3D9
	IDirect3D9 *&d3d = *reinterpret_cast<IDirect3D9 **>( 0xC97C20 );
	/// Указатель на D3DPRESENT_PARAMETERS. Там всякие нештяки типа разрешения экрана
	D3DPRESENT_PARAMETERS &params = *reinterpret_cast<D3DPRESENT_PARAMETERS *>( 0xC9C040 );
#endif

#if defined(FULL_DX_HOOK) && __has_include(<ProxyDX9/ProxyDX9.h>)
	/**
	 * @brief Указатель на proxyIDirect3DDevice9
	 * @detail Данный класс является хуком над оригинальным IDirect3DDevice9.
	 * Он используется для решистрации различных примитивов и получения оригинального указателя
	 * IDirect3DDevice9. Данный класс так же позволяет перенаправлять методы IDirect3DDevice9 в другие классы,
	 * используя систему сигналов и слотов. Если вы не хотите, что бы после вызова вашего хука, вызывался
	 * оригинальный метод (например, если вы вызываете его сами), то дергайте метод d3d9_hook
	 */
	hookIDirect3DDevice9 *DirectX = nullptr;
#endif

#if defined(USE_DRAW_HOOK) && __has_include(<DrawHook/DrawHook.h>)
	/// Указатель на хук рисования
	DrawHook *draw = nullptr;
#endif

#if __has_include(<SRCursor/SRCursor.h>)
	/// Объект для управления вызовом курсора на экран. Не зависит от версии сампа
	SRCursor *cursor = nullptr;
#endif

#if __has_include(<SREvents/SREvents.h>)
	/**
	 * @brief Указатель на класс обработки различных событий
	 * @details Данный класс позволяет перехватывать события окна игры, выполнять свой код во время работы
	 * mainloop и scriptloop, обрабатывать нажатия клавиш и хукать события RakNet
	 */
	SREvents *events = nullptr;
#endif
} g_class;

/// Название проекта, которое было указано при разворачивание проекта из шаблона
extern const std::string_view PROJECT_NAME;

/**
 * @brief Выводит окно с сообщениемю
 * @detail Является оберткой над MessageBoxA, для более удобного вывода сообщений.
 * @param[in] text Текст сообщения.
 * @param[in] title Заголовок окна с сообщением.
 * @param[in] type Тип окна.
 * @return код завершения окна (нажатая кнопка).
 */
int MessageBox( std::string_view text, std::string_view title = PROJECT_NAME, UINT type = MB_OK );

#endif // LOADER_H
