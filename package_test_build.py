import os
import shutil
import zipfile

# Copiamos el VPK original
src_vpk = "build/zenonia_3.vpk"
dest_vpk = "build/zenonia3_test_build.vpk"

print(f"Creating {dest_vpk}...")
shutil.copyfile(src_vpk, dest_vpk)

# Archivos a anadir
# DRAWABLE_ASSETS de manage_vita.py
DRAWABLE_ASSETS = [
    "ui_logo_gamevil.png",
    "ui_title_bg_nate.png",
    "ui_title_logo5.png",
    "ui_menu_back0.png",
    "ui_menu_back1.png",
    "ui_menu_newgame.png",
    "ui_menu_continue.png",
    "ui_menu_options.png",
    "ui_menu_help.png",
    "ui_menu_about.png",
    "ui_menu_community.png",
    "ui_about_bg.png",
    "ui_help_bg.png",
    "ui_menu_back.png",
    "reply_page_back_e.png",
    "button_write_01_global.png",
    "button_later_01_global.png",
]

with zipfile.ZipFile(dest_vpk, 'a', zipfile.ZIP_DEFLATED) as zf:
    # 1. libgameDSO.so
    print("Adding libgameDSO.so...")
    zf.write("zenonia3/lib/armeabi/libgameDSO.so", "ux0/data/zenonia3/libgameDSO.so")

    # 2. assets/
    print("Adding assets/...")
    for root, dirs, files in os.walk("zenonia3/assets"):
        for file in files:
            file_path = os.path.join(root, file)
            # zenonia3/assets/ -> ux0/data/zenonia3/assets/
            rel_path = os.path.relpath(file_path, "zenonia3/assets")
            zf.write(file_path, f"ux0/data/zenonia3/assets/{rel_path}")

    # 3. html/ (tambien estan en assets/html, los copiamos a ux0/data/zenonia3/html/ como espera el port)
    print("Adding html/...")
    if os.path.exists("zenonia3/assets/html"):
        for root, dirs, files in os.walk("zenonia3/assets/html"):
            for file in files:
                file_path = os.path.join(root, file)
                rel_path = os.path.relpath(file_path, "zenonia3/assets/html")
                zf.write(file_path, f"ux0/data/zenonia3/html/{rel_path}")

    # 4. drawable/ (solo los usados)
    print("Adding drawable/...")
    for file in DRAWABLE_ASSETS:
        file_path = os.path.join("zenonia3/res/drawable", file)
        if os.path.exists(file_path):
            zf.write(file_path, f"ux0/data/zenonia3/drawable/{file}")
        else:
            print(f"Warning: {file_path} not found")

    # 5. sound/
    print("Adding sound/...")
    for root, dirs, files in os.walk("ux0_data/zenonia3/sound"):
        for file in files:
            file_path = os.path.join(root, file)
            rel_path = os.path.relpath(file_path, "ux0_data/zenonia3")
            zf.write(file_path, f"ux0/data/zenonia3/{rel_path}")  # ux0/data/zenonia3/sound/...

print("Done!")
