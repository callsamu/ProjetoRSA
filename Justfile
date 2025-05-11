run: build
	./builddir/rsa

build:
	meson compile -C ./builddir

setup:
	meson setup builddir
