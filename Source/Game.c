// virtual file

struct {
    int (*LoadConfig)();
    int (*LoadEnv)();
    int (*LoadGraphics)();
    int (*LoadRes)();
    int (*PrepareOutput)();
    int (*PrepareUI)();
    int (*PrepareScene)();
    int (*RunGame)();
    int (*RunUpdate)();
    int (*SaveConfig)();
    int (*SceneChange)(int);
    int (*ExitGame)();
    int (*CleanUp)();
} GlobalGameFunc;

int start()
{
    return 0;
}
