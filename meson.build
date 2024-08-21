project('goldsrc', ['c', 'cpp'],
    version: '0',
    meson_version : '>= 0.49',
    default_options : ['buildtype=debugoptimized','backend_startup_project=client'],
)

cpp = meson.get_compiler('cpp')
cc = meson.get_compiler('c')

if cpp.get_id() == 'msvc'
    res_ext = '.res'
    wrc = find_program('rc')
    wrc_generator = generator(wrc,
        output    : [ '@BASENAME@' + res_ext ],
        arguments : [ '/fo', '@OUTPUT@', '@INPUT@' ],
    )
    add_global_arguments('/fpcvt:BC', language : 'c')
    add_global_arguments('/fpcvt:BC', language : 'cpp')

    add_project_link_arguments('/LARGEADDRESSAWARE', language : ['c', 'cpp'])
else
    res_ext = '.o'
    wrc = find_program('windres')
    wrc_generator = generator(wrc,
        output    : [ '@BASENAME@' + res_ext ],
        arguments : [ '-i', '@INPUT@', '-o', '@OUTPUT@' ],
    )
endif

output_dir = meson.source_root() + '/../game'

goldsrc_includes = include_directories([
    'common',
    'public',
    'external',
])

winmm_dep = cc.find_library('winmm')
shlwapi_dep = cc.find_library('shlwapi')
opengl32_dep = cc.find_library('opengl32')
wsock32_dep = cc.find_library('wsock32')
ws2_32_dep = cc.find_library('ws2_32')
wininet_dep = cc.find_library('wininet')
dxguid_dep = cc.find_library('dxguid')
ddraw_dep = cc.find_library('ddraw')
SDL2_dep = cc.find_library('SDL2', dirs : meson.source_root() + '/lib/public')
steam_api_dep = cc.find_library('steam_api', dirs : meson.source_root() + '/lib/public')
game_controls_dep = cc.find_library('game_controls', dirs : meson.source_root() + '/lib/public')
vgui_dep = cc.find_library('vgui', dirs : meson.source_root() + '/utils/vgui/lib/win32_vc16')
vgui2_dep = cc.find_library('vgui2', dirs : meson.source_root() + '/utils/vgui2/lib/win32_vc16')
vgui_controls_dep = cc.find_library('vgui_controls', dirs : meson.source_root() + '/lib/public')

goldsrc_defines = [
    '-D_CRT_SECURE_NO_WARNINGS',

    '-DWIN32',
    '-DQUAKE2',
    '-DQUIVER',
    '-DHL1',
    '-DVALVE_DLL',
]
add_project_arguments(cpp.get_supported_arguments(goldsrc_defines), language: 'cpp')
add_project_arguments(cpp.get_supported_arguments(goldsrc_defines), language: 'c')

# mods
subdir('ricochet')
subdir('dmc')

# hl1
subdir('dlls') 
subdir('cl_dll')

