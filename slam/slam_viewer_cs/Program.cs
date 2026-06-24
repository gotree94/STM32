namespace SlamViewer;

static class Program
{
    [STAThread]
    static void Main(string[] args)
    {
        ApplicationConfiguration.Initialize();
        string? path = args.Length > 0 ? args[0] : null;
        Application.Run(new MainForm(path));
    }
}
