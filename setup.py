from distutils.core import setup, Extension
def main():
    setup(
        name="hook",
        version="1.0.0",
        description="Simple Python interface for Window Mouse Hook and Window KeyBoard Hook",
        author="werido",
        author_email="359066432@qq.com",
        ext_modules=[Extension("hook", ["hook.c"],language="c")]

    )


if __name__ == "__main__":
    main()
