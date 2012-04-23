/*See LICENSE file for copyright and license details.*/

/*TODO: add colorFG, colorBG*/
typedef struct {
  int id;
  V2i pos;
  V2i size;
  GLuint texture_id;
  char *text;
  TTF_Font *f;
  void (*callback)(void);
} Button;

TTF_Font *open_font(char *font_name, int size);

Button *v2i_to_button(V2i pos);
void add_button(TTF_Font *f, int id, V2i pos, char *text, void (*callback)(void));
void draw_buttons(void);
void change_button_text(Button *b, char *text);
void change_button_text_by_id(int id, char *text);
