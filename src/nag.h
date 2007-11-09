void closeNagBtnCbk(GtkButton* button, GtkWidget* parentWindow);
void enterRegistrationCodeCbk(GtkButton* button, GtkWidget* parentWindow);
void getRegistrationCodeCbk(GtkButton* button, gpointer user_data);
gboolean nagCountdown(gpointer data);
gboolean preventClosingNagWindowCbk(GtkWidget* widget, GdkEvent  *event,
                                    gpointer user_data);
void showNagScreen(void);
