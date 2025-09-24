import os
import shutil
import subprocess
from pathlib import Path

def make_animation():
    base_dir = Path(__file__).resolve().parent
    screenshots_dir = base_dir.parent / "hw1" / "screenshots"
    gen_dir = base_dir / "gen"
    gen_dir.mkdir(exist_ok=True)

    # get latest run
    run_folders = sorted(screenshots_dir.glob("RUN_*"), key=os.path.getmtime)
    if not run_folders:
        raise RuntimeError("No RUN_* folders found in screenshots/")
    src = run_folders[-1]  # latest
    print(f"📂 Using source folder: {src}")

    # generate folder
    i = 0
    while (gen_dir / f"animation_ver_{i}").exists():
        i += 1
    out_dir = gen_dir / f"animation_ver_{i}"
    out_dir.mkdir(parents=True)

    images = sorted(src.glob("*.jpg"))
    if not images:
        raise RuntimeError(f"No .jpg images found in {src}")
    if len(images) < 300:
        print(f"⚠️ Only {len(images)} images found, expected 300.")

    # copy & rename to fit requirement
    for idx, img in enumerate(images[:300]):
        new_name = f"{idx:03d}.jpg"
        shutil.copy(img, out_dir / new_name)

    # make video
    video_path = out_dir / "animation.mp4"
    cmd = [
        "ffmpeg",
        "-y",
        "-framerate", "15",
        "-i", str(out_dir / "%03d.jpg"),
        "-c:v", "libx264",
        "-pix_fmt", "yuv420p",
        str(video_path)
    ]
    # uses ffmpeg
    # brew install ffmpeg
    print("▶️ Running ffmpeg...")
    subprocess.run(cmd, check=True)

    print(f"✅ Animation ready: {video_path}")

if __name__ == "__main__":
    make_animation()
