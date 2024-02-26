/*
 * Разработать упрощённый текстовый редактор, который поддерживает следующий набор команд:
 * - Перемещение курсора влево (Left) и вправо (Right) на одну позицию;
 * - Ввод символа в текущую позицию курсора (Insert);
 * - Копирование фрагмента текста, начинающегося в текущей позиции курсора, в буфер обмена (Copy);
 * - Вырезание фрагмента текста — аналогично копированию с последующим удалением скопированных символов из текста (Cut);
 * - Вставка содержимого буфера обмена в текущую позицию курсора (Paste).
 * - метод GetText(), позволяющий получить текущее содержимое редактора в виде строки
 *
 * Для команд действуют правила, определяющие их эффект:
 * - Если редактор содержит текст длиной n символов, курсор может находиться в одной из (n + 1) возможных позиций.
 * - Обозначим курсор вертикальной чертой | и будем использовать это обозначение дальше.
 * - В тексте abc курсор может быть в позициях: |abc, a|bc, ab|c, abc|.
 * Поэтому команда Left не имеет эффекта, когда курсор расположен в начале текста, а Right не имеет эффекта,
 * когда курсор находится в конце. Ни Left, ни Right не имеют эффекта, когда редактор не содержит текста.
 * - Введённый символ располагается в позиции курсора, сдвигая курсор и весь текст справа от него на одну позицию вправо.
 * - Аналогично при вставке фрагмента длиной n курсор и текст справа от него смещаются на n позиций вправо.
 *
 * Буфер обмена изначально пуст. Вставка пустого фрагмента не имеет эффекта.
 * Содержимое буфера не сбрасывается после вставки, а остаётся неизменным до следующей команды Copy или Cut.
 * Копирование или вырезание фрагмента нулевой длины не оказывает влияния на текст, но опустошает буфер обмена.
 * Курсор не смещается ни при копировании, ни при вырезании текста.
 */
#include <iostream>
#include <iterator>
#include <list>
#include <string>

class Editor {
public:
    Editor() = default;

    void Left(); // сдвинуть курсор влево

    void Right(); // сдвинуть курсор вправо

    void Insert(char token); // вставить символ token

    void Cut(size_t tokens = 1); // вырезать не более tokens символов, начиная с текущей позиции курсора

    void Copy(size_t tokens = 1); // cкопировать не более tokens символов, начиная с текущей позиции курсора

    void Paste(); // вставить содержимое буфера в текущую позицию курсора

    std::string GetText() const; // получить текущее содержимое текстового редактора
private:
    std::list<char> text_;
    std::list<char>::iterator cursor_ = text_.begin();
    std::list<char> exch_buf_;
};

void Editor::Left() {
    if(cursor_ != text_.begin()) {
        --cursor_;
    }
}

void Editor::Right() {
    if(cursor_ != text_.end()) {
        ++cursor_;
    }
}

void Editor::Insert(char token) {
    text_.insert(cursor_, token);
    Right();
}

void Editor::Cut(size_t tokens) {
    exch_buf_.clear();
    for(size_t i = 0; (i != tokens) && (cursor_ != text_.end()); ++i) {
        exch_buf_.push_back(*cursor_);
        auto old_cursor = cursor_;
        Right();
        text_.erase(old_cursor);
    }
}

void Editor::Copy(size_t tokens) {
    exch_buf_.clear();
    for (size_t i = 0; (i != tokens) && (cursor_ != text_.end()); ++i) {
        exch_buf_.push_back(*cursor_);
        Right();
    }
}

void Editor::Paste() {
    text_.insert(cursor_, exch_buf_.begin(), exch_buf_.end());
}

std::string Editor::GetText() const {
    std::string result = "";
    for(char c : text_) {
        result.push_back(c);
    }
    return result;
}

using namespace std::literals;

int main() {
    Editor editor;
    const std::string text = "hello, world"s;
    for (char c : text) {
        editor.Insert(c);
    }
    // Текущее состояние редактора: `hello, world|`
    for (size_t i = 0; i < text.size(); ++i) {
        editor.Left();
    }
    // Текущее состояние редактора: `|hello, world`
    editor.Cut(7);
    // Текущее состояние редактора: `|world`
    // в буфере обмена находится текст `hello, `
    for (size_t i = 0; i < 5; ++i) {
        editor.Right();
    }
    // Текущее состояние редактора: `world|`
    editor.Insert(',');
    editor.Insert(' ');
    // Текущее состояние редактора: `world, |`
    editor.Paste();
    // Текущее состояние редактора: `world, hello, |`
    editor.Left();
    editor.Left();
    // Текущее состояние редактора: `world, hello|, `
    editor.Cut(3); // Будут вырезаны 2 символа
    // Текущее состояние редактора: `world, hello|`
    std::cout << editor.GetText();
    return 0;
}