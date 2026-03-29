# axmol_ports — Top-level build system for AmigaOS4
#
# Usage:
#   make lom          Build The Lords of Midnight
#   make ddr          Build Doomdark's Revenge
#   make all          Build both games
#   make axmol        Build engine library only
#   make openal       Build OpenAL Soft only
#   make glfw         Build GLFW only
#   make vulkan-sdk   Build Vulkan SDK only
#   make clean        Clean all build artifacts
#   make strip        Strip both game binaries

DOCKER_IMAGE = clib4-dev:os4-gcc11
DOCKER_RUN   = docker run --rm -v "$(CURDIR):/work" -w /work $(DOCKER_IMAGE)
TOOLCHAIN    = /work/axmol/cmake/amigaos4.cmake
JOBS         = $(shell nproc)

.PHONY: all lom ddr axmol openal glfw vulkan-sdk clean strip

all: lom ddr

# ── Phase 1: Vulkan SDK ──────────────────────────────────────────────
vulkan-sdk: vulkan-sdk/lib/libvulkan_loader.a

vulkan-sdk/lib/libvulkan_loader.a:
	$(DOCKER_RUN) sh -c "cd /work/vulkan-sdk && make -f Makefile all"

# ── Phase 2: GLFW ────────────────────────────────────────────────────
glfw: glfw/build/src/libglfw3.a

glfw/build/src/libglfw3.a:
	@mkdir -p glfw/build
	$(DOCKER_RUN) sh -c "\
		cd /work/glfw/build && \
		cmake ../src \
			-DCMAKE_TOOLCHAIN_FILE=/work/glfw/amigaos4.cmake \
			-DGLFW_BUILD_OS4=ON \
			-DBUILD_SHARED_LIBS=OFF \
			-DGLFW_BUILD_EXAMPLES=OFF \
			-DGLFW_BUILD_TESTS=OFF \
			-DGLFW_BUILD_DOCS=OFF && \
		make -j$(JOBS)"

# ── Phase 3: OpenAL Soft ─────────────────────────────────────────────
openal: openal-soft/build/libopenal.a

openal-soft/build/libopenal.a:
	@mkdir -p openal-soft/build
	$(DOCKER_RUN) sh -c "\
		cd /work/openal-soft/build && \
		cmake ../src \
			-DCMAKE_TOOLCHAIN_FILE=/work/openal-soft/amigaos4.cmake \
			-DLIBTYPE=STATIC \
			-DALSOFT_BACKEND_AHI=ON \
			-DALSOFT_BACKEND_WAVE=OFF \
			-DALSOFT_BACKEND_PIPEWIRE=OFF \
			-DALSOFT_BACKEND_PULSEAUDIO=OFF \
			-DALSOFT_BACKEND_ALSA=OFF \
			-DALSOFT_BACKEND_OSS=OFF \
			-DALSOFT_BACKEND_SNDIO=OFF \
			-DALSOFT_BACKEND_JACK=OFF \
			-DALSOFT_BACKEND_OBOE=OFF \
			-DALSOFT_BACKEND_OPENSL=OFF \
			-DALSOFT_BACKEND_PORTAUDIO=OFF \
			-DALSOFT_BACKEND_SDL2=OFF \
			-DALSOFT_EXAMPLES=OFF \
			-DALSOFT_UTILS=OFF \
			-DALSOFT_TESTS=OFF \
			-DALSOFT_INSTALL=OFF && \
		make -j$(JOBS)"

# ── Phase 4: axmol engine ────────────────────────────────────────────
axmol: axmol/build/libaxmol.a

axmol/build/libaxmol.a: glfw/build/src/libglfw3.a openal-soft/build/libopenal.a
	@mkdir -p axmol/build
	$(DOCKER_RUN) sh -c "\
		cd /work/axmol/build && \
		cmake .. -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) && \
		make -j$(JOBS)"

# ── Phase 5: Games ───────────────────────────────────────────────────
lom: lords-of-midnight/build-lom/LordsOfMidnight
ddr: lords-of-midnight/build-ddr/DoomdarksRevenge

lords-of-midnight/build-lom/LordsOfMidnight: axmol/build/libaxmol.a
	@mkdir -p lords-of-midnight/build-lom
	$(DOCKER_RUN) sh -c "\
		cd /work/lords-of-midnight/build-lom && \
		cmake .. -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) -DTME=LOM && \
		make -j$(JOBS)"

lords-of-midnight/build-ddr/DoomdarksRevenge: axmol/build/libaxmol.a
	@mkdir -p lords-of-midnight/build-ddr
	$(DOCKER_RUN) sh -c "\
		cd /work/lords-of-midnight/build-ddr && \
		cmake .. -DCMAKE_TOOLCHAIN_FILE=$(TOOLCHAIN) -DTME=DDR && \
		make -j$(JOBS)"

# ── Utilities ─────────────────────────────────────────────────────────
strip: lom ddr
	$(DOCKER_RUN) sh -c "\
		ppc-amigaos-strip -R.comment /work/lords-of-midnight/build-lom/LordsOfMidnight && \
		ppc-amigaos-strip -R.comment /work/lords-of-midnight/build-ddr/DoomdarksRevenge"
	@ls -lh lords-of-midnight/build-lom/LordsOfMidnight lords-of-midnight/build-ddr/DoomdarksRevenge

clean:
	rm -rf vulkan-sdk/build vulkan-sdk/lib
	rm -rf glfw/build
	rm -rf openal-soft/build
	rm -rf axmol/build
	rm -rf lords-of-midnight/build-lom lords-of-midnight/build-ddr lords-of-midnight/build
