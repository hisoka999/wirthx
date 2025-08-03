program gtkeditor;
uses gtk4;

procedure activate (var app: GtkApplication;
          user_data: gpointer);
var
    window : GtkWidget;
begin
  window := gtk_application_window_new (app);
  gtk_window_set_title (window, "Window");
  gtk_window_set_default_size (window, 200, 200);
  gtk_window_present (window);
end;


var
    app: GtkApplication;
    status : integer;
    argc : integer = 0;
    argv :^pchar;
    appname : string = 'org.gtk.example';
begin 
  app := gtk_application_new (pchar(appname), 0);
  g_signal_connect (app, 'activate', activate, 0);
  status := g_application_run (app, argc, argv);
  g_object_unref (app);
  halt(status);
end.