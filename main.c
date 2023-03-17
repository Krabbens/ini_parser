#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <locale.h>
#include <inttypes.h>

// C -> 🤡

/*
    dla czytelności deklarujemy typ any 
    (czy jest czytelniejszy? to już kwestia gustu)
    pod którym kryje się typ void* przechowujący wskaźnik
    będziemy używać go do przechowywania dowolnego typu danych
    które będziemy rzutować zależnie od tego co znajduje się
    w section.value_types
*/

typedef void* any;

typedef struct string {
    char* data;
    int max_size;
    int len;
} string;

typedef struct v_string {
    string** data;
    int max_size;
    int len;
} v_string;

typedef struct section {
    string* key;
    string** value_names;
    string** value_types;
    any* values;
    int len;
    int max_size;
} section;

typedef struct ini_file {
    section** sections;
    int len;
    int max_size;
} ini_file;

typedef struct {
    char* key;
    char* value;
} key_value_pair;

typedef struct {
    key_value_pair* left_operand;
    char* operator;
    key_value_pair* right_operand;
} equation;

void init_str(string* _s) {
    // ustawiamy zmienne początkowe dla stringa
    _s->max_size = 1;
    _s->data = (char*)malloc(sizeof(char));
    _s->len = 0;
}

void expand_str(string* _s) {
    /*
    - zwiększamy rozmiar tablicy dwukrotnie
    - realloc przydziela dodatkową pamięć
    - jeśli udało się przydzielić pamięć to ją przypisujemy
    */
    _s->max_size *= 2;
    char* _temp = (char*)realloc(_s->data, sizeof(char) * _s->max_size);
    if (_temp) _s->data = _temp;
}

void add_char_to_str(string* _s, char _c) {
    /*
    - jeśli brakuje miejsca w tablicy to ją rozszerzamy
    - następnie wpisujemy znak do tablicy
    - po wpisaniu zwiększamy zmienną która wskazuje nam zapełnienie tablicy
    */
    if (_s->len + 1 > _s->max_size) expand_str(_s);
    _s->data[_s->len++] = _c;
}

void add_str_to_str(string* _s1, string* _s2) {
    /*
    - wpisujemy wszystkie znaki z jednego stringa do drugiego
    - da się to zrobić też przez memcpy, ale tak jest prościej
    */
    for (int _it = 0; _it < _s2->len; ++_it) {
        add_char_to_str(_s1, _s2->data[_it]);
    }
}

void add_cstr_to_str(string* _s1, char* _s2) {
    /*
    - to samo co w add_str_to_str ale dla tablicy charów
    */
    int _it = 0;
    while (_s2[_it] != '\0') {
        add_char_to_str(_s1, _s2[_it++]);
    }
}

void init_section(section* _s) {
    // inicjalizujemy zmienne dla sekcji
    _s->len = 0;
    _s->max_size = 1;
    _s->value_names = (string**)malloc(sizeof(string*));
    _s->value_types = (string**)malloc(sizeof(string*));
    _s->values = (any*)malloc(sizeof(any));
}

void expand_section(section* _s) {
    /*
    - rozszerzamy sekcje gdy brakuje miejsca
    - analogicznie do expand_str tylko że jest więcej zmiennych
    */
    _s->max_size *= 2;
    string** _temp_value_names = (string**)realloc(_s->value_names, _s->max_size * sizeof(string*));
    string** _temp_value_types = (string**)realloc(_s->value_types, _s->max_size * sizeof(string*));
    any* _temp_values = (any*)realloc(_s->values, _s->max_size * sizeof(any));
    if (_temp_value_names && _temp_value_types && _temp_values) {
        _s->value_names = _temp_value_names;
        _s->value_types = _temp_value_types;
        _s->values = _temp_values;
    }
}

void add_key_value(section* _s, string* _name, string* _type, void* _value) {
    /*
    - tak samo jak w dodawaniu chara do stringa
    - sprawdzamy czy się mieści, jeśli nie to rozszerzamy
    - dodajemy nową parę klucz-wartość a na końcu zwiększamy licznik
    */
    if (_s->len + 1 > _s->max_size) expand_section(_s);
    _s->value_names[_s->len] = _name;
    _s->value_types[_s->len] = _type;
    _s->values[_s->len++] = _value;
}

void init_ini_file(ini_file* _i) {
    // inicjalizacja zmiennych początkowych
    _i->len = 0;
    _i->max_size = 1;
    _i->sections = (section**)malloc(sizeof(section*));
}

void expand_ini_file(ini_file* _i) {
    // analogicznie do poprzednich expandów
    _i->max_size *= 2;
    section** _temp = (section**)realloc(_i->sections, _i->max_size * sizeof(section*));
    if (_temp) _i->sections = _temp;
}

void add_section(ini_file* _i, section* _s) {
    // analogicznie do poprzedniego dodawania elementów
    if (_i->len + 1 > _i->max_size) expand_ini_file(_i);
    _i->sections[_i->len++] = _s;
}

void init_vstr(v_string* _v) {
    // inicjalizujemy wektor stringów
    _v->len = 0;
    _v->max_size = 1;
    _v->data = (string**)malloc(sizeof(string*));
}

void expand_vstr(v_string* _v) {
    // analogicznie do poprzednich expandów
    _v->max_size *= 2;
    string** _temp = (string**)realloc(_v->data, sizeof(string*) * _v->max_size);
    if (_temp) _v->data = _temp;
}

void append_vstr(v_string* _v, string* _s) {
    // analogicznie do poprzedniego dodawania elementów
    if (_v->len + 1 > _v->max_size) expand_vstr(_v);
    _v->data[_v->len++] = _s;
}

char* str_to_cstr(string* _s) {
    // konwersja stringa na tablicę charów
    char* _c = (char*)malloc(sizeof(char) * _s->len + 1);
    if (_c) {
        memcpy(_c, _s->data, _s->len * sizeof(char));
        _c[_s->len] = '\0';
    }
    return _c;
}

void split_string(const char* input, char** key, char** value) {
    /*
    - dzielimy stringa na klucz i wartość
    - najpierw tworzymy kopię stringa, żeby nie zmieniać oryginału
    - następnie dzielimy go na klucz i wartość
    - usuwamy białe znaki z początku i końca klucza
    - usuwamy białe znaki z początku wartości
    */
    char* input_copy = strdup(input);
    char* delimiter = "=";
    char* token;

    token = strtok(input_copy, delimiter);
    while (isspace(*token)) {
        token++;
    }
    *key = strdup(token);

    token = strtok(NULL, "");
    while (isspace(*token)) {
        token++;
    }
    *value = strdup(token);

    int len = strlen(*key);
    while (len > 0 && isspace((*key)[len - 1])) {
        (*key)[len - 1] = '\0';
        len--;
    }

    free(input_copy);
}

char* get_type(char* data) {
    /*
    - sprawdzamy czy wartość jest liczbą
    - jeśli tak to zwracamy "number"
    - jeśli nie to "string"
    */
    int is_number = 1;
    int len = strlen(data);
    for (int i = 0; i < len; i++) {
        if (!isdigit(data[i])) {
            is_number = 0;
            break;
        }
    }

    if (is_number) {
        return strdup("number");
    }
    else {
        return strdup("string");
    }
}

char compare_str_cstr(string* _s, const char* _c) {
    /*
    - konwertujemy stringa na tablicę charów
    - porównujemy tablicę charów z tablicą charów
    - zwalniamy pamięć
    */
    char* _t = str_to_cstr(_s);
    char _cmp = strcmp(_t, _c);
    free(_t);
    return _cmp;
}

char compare_str_str(string* _s1, string* _s2) {
    /*
    - konwertujemy stringi na tablice charów
    - porównujemy tablice charów
    - zwalniamy pamięć
    */
    char* _t1 = str_to_cstr(_s1);
    char* _t2 = str_to_cstr(_s2);
    char _cmp = strcmp(_t1, _t2);
    free(_t1);
    free(_t2);
    return _cmp;
}

void handle_get_data(char* _input, char** _key, char** _value) {
    /*
    - tworzymy kopię stringa, żeby nie zmieniać oryginału
    - dzielimy go na klucz i wartość
    */
    char* input_copy = strdup(_input);
    char* delimiter = ".";
    char* token;

    token = strtok(input_copy, delimiter);
    *_key = strdup(token);

    token = strtok(NULL, "");
    *_value = strdup(token);

    free(input_copy);
}

void split_argument(char* _arg, equation* equation) {
    /*
    - tworzymy kopię stringa, żeby nie zmieniać oryginału
    - dzielimy go na lewą i prawą stronę równania
    - dzielimy lewą stronę równania na klucz i wartość
    - dzielimy prawą stronę równania na klucz i wartość
    - zapisujemy operator
    */
    char* arg = strdup(_arg);
    char* token;
    int i = 0;

    key_value_pair* left_operand = malloc(sizeof(key_value_pair));
    if (!left_operand) return;
    key_value_pair* right_operand = malloc(sizeof(key_value_pair));
    if (!right_operand) return;
    char* operator = malloc(sizeof(char)+1);

    token = strtok(arg, ".");
    while (token != NULL) {
        if (i == 0) {
            left_operand->key = strdup(token);
            token = strtok(NULL, " ");
        }
        else if (i == 1) {
            left_operand->value = strdup(token);
            token = strtok(NULL, " ");
        }
        else if (i == 2) {
            // usuwamy białe znaki z początku i końca operatora
            while (isspace(*token)) {
                token++;
            }
            int len = strlen(token);
            while (len > 0 && isspace(token[len - 1])) {
                token[len - 1] = '\0';
                len--;
            }
            *operator = *token;
            *(operator+1) = '\0';
            token = strtok(NULL, " .");
        }
        else if (i == 3) {
            right_operand->key = strdup(token);
            token = strtok(NULL, " ");
        }
        else if (i == 4) {
            right_operand->value = strdup(token);
            token = strtok(NULL, " ");
        }
        i++;
    }

    equation->left_operand = left_operand;
    equation->operator = operator;
    equation->right_operand = right_operand;
    free(arg);
    free(token);
}

section* find_arg(ini_file* _i, char* _key, char* _value, int* _out_idx) {
    /*
    - szukamy sekcji o podanym kluczu
    - szukamy wartości o podanej nazwie w sekcji
    - zwracamy indeks wartości w sekcji
    */
    char _found_section = 0;
    for (int i = 0; i < _i->len; i++) {
        if (!compare_str_cstr(_i->sections[i]->key, _key)) {
            _found_section = 1;
            for (int j = 0; j < _i->sections[i]->len; j++) {
                if (!compare_str_cstr(_i->sections[i]->value_names[j], _value)) {
                    *_out_idx = j;
                    return _i->sections[i];
                }
            }
        }
    }
    if (!_found_section) {
        printf("Failed to find section [%s]\n", _key);
        return NULL;
    } else {
        printf("Failed to find key \"%s\" in section [%s]\n", _value, _key);
        return NULL;
    }
}

void parse_ini(ini_file* _i, char** _args, int _arg_len) {
    /*
    - wczytujemy plik i następnie tworzymy tablicę stringów
    - do tablicy stringów wpisujemy kolejno wszystkie linie spełniające warunki zadania
    - nie wpisujemy komentarzy
    */
    char* _p = _args[1];
    FILE* _f = fopen(_p, "r");
    if (!_f) {
        printf("error - can't open file\n");
        free(_i->sections);
        return;
    }
    char _t;
    char _first_char = '\0';
    char _is_first = 1;
    v_string _v;
    string* _s = NULL;
    init_vstr(&_v);
    while ((_t = fgetc(_f)) != EOF) {
        if (_is_first) {
            _first_char = _t;
            _is_first = 0;
            _s = (string*)malloc(sizeof(string));
            init_str(_s);
        }
        if (_t == '\n') {
            _is_first = 1;
            char err = 0;
            if (_s && _s->len == 0 && _s->data) {
                free(_s->data);
                free(_s);
                err = 1;
            }
            if (_first_char == '\n' || err) continue;
            else {
                for (int i = 0; i < _s->len; i++) {
                    if (!(isalnum(_s->data[i]) || _s->data[i] == '[' || _s->data[i] == '=' || _s->data[i] == ']' || _s->data[i] == '-' || _s->data[i] == ' ')) {
                        char* _errc = str_to_cstr(_s);
                        printf("error - found invalid character in %s -> %c\n", _errc, _s->data[i]);
                        free(_errc);
                        free(_s->data);
                        free(_s);
                        
                        err = 1;
                        break;
                    }
                }
            }
            if (!err) {
                append_vstr(&_v, _s);
            }
            continue;
        }
        
        if (_first_char == ';') continue;
        add_char_to_str(_s, _t);
    }
    fclose(_f);
    // for (int i = 0; i < _v.len; i++) {
    //     char* temp = str_to_cstr(_v.data[i]);
    //     printf("%s\n", temp);
    //     free(temp);
    // }

    /*
    - tworzymy tymczasową sekcję
    - iterujemy po tablicy stringów
    - jeśli napotkamy sekcję to dodajemy ją do pliku
    - jeśli napotkamy wartość to dodajemy ją do sekcji
    */
    section* _temp_sec = (section*)malloc(sizeof(section));
    init_section(_temp_sec);
    char _is_next_section = 0;
    char _found_section = 0;
    for (int i = 0; i < _v.len; i++) {
        if (_v.data[i]->data[0] == '[') {
            // sprawdzamy czy to początek sekcji
            if (_is_next_section) {
                add_section(_i, _temp_sec);
                _temp_sec = (section*)malloc(sizeof(section));
                init_section(_temp_sec);
            }
            string* _name = (string*)malloc(sizeof(string));
            init_str(_name);
            for (int j = 1; j < _v.data[i]->len-1; j++) {
                add_char_to_str(_name, _v.data[i]->data[j]);
            }
            if (_temp_sec)
                _temp_sec->key = _name;
            _is_next_section = 1;
            _found_section = 1;
        }
        else {
            if (_found_section) {
                string* _key = (string*)malloc(sizeof(string));
                string* _type = (string*)malloc(sizeof(string));
                init_str(_key);
                init_str(_type);
                char* _input = str_to_cstr(_v.data[i]);
                char* _c_key = 0;
                char* _c_val = 0;
                split_string(_input, &_c_key, &_c_val);
                add_cstr_to_str(_key, _c_key);

                char* _c_type = get_type(_c_val);
                add_cstr_to_str(_type, _c_type);
                
                any _value;
                if (!compare_str_cstr(_type, "string")) {
                    _value = (any)_c_val;
                }
                else {
                    _value = (any)(intptr_t)atoi(_c_val);
                    free(_c_val);
                }

                free(_input);
                free(_c_key);
                free(_c_type);

                add_key_value(_temp_sec, _key, _type, _value);
            }
        }
    }
    for (int i = 0; i < _v.len; i++) {
        free(_v.data[i]->data);
        free(_v.data[i]);
    }
    free(_v.data);
    add_section(_i, _temp_sec);

    // for (int i = 0; i < _i->len; i++) {
    //     char* _temp = str_to_cstr(_i->sections[i]->key);
    //     printf("%s\n", _temp);
    //     free(_temp);
    //     for (int j = 0; j < _i->sections[i]->len; j++) {
    //         _temp = str_to_cstr(_i->sections[i]->value_names[j]);
    //         printf("key -> %s\n", _temp);
    //         free(_temp);
    //         _temp = str_to_cstr(_i->sections[i]->value_types[j]);
    //         printf("type -> %s\n", _temp);
    //         free(_temp);
    //         if (compare_str_cstr(_i->sections[i]->value_types[j], "string") == 0)
    //             printf("value -> %s\n", (char*)_i->sections[i]->values[j]);
    //         else
    //             printf("value -> %ld\n", (intptr_t)_i->sections[i]->values[j]);
    //         printf("\n");
    //     }
    // }
    
    /*
    - jeśli są 3 argumenty to szukamy wartości
    - jeśli są 4 argumenty to szukamy wartości i wykonujemy operację
    */

    if (_arg_len == 3) {
        char* _key;
        char* _value;
        handle_get_data(_args[2], &_key, &_value);
        int _idx;
        section* _found_section = find_arg(_i, _key, _value, &_idx);
        
        if (_found_section) {
            any _found_value = _found_section->values[_idx];
            if (_found_value) {
                if (!compare_str_cstr(_found_section->value_types[_idx], "string"))
                    printf("%s\n", (char*)_found_value);
                else
                    printf("%d\n", (int)(intptr_t)_found_value);
            }
        }
    }
    if (_arg_len == 4 && !strcmp(_args[2], "expression")) {
        equation _e;
        split_argument(_args[3], &_e);
        int _l_idx, _r_idx;
        section* _l_section = find_arg(_i, _e.left_operand->key, _e.left_operand->value, &_l_idx);
        section* _r_section = find_arg(_i, _e.right_operand->key, _e.right_operand->value, &_r_idx);
        if (_l_section && _r_section) {
            if (!compare_str_str(_l_section->value_types[_l_idx], _r_section->value_types[_r_idx]))
            {
                if (!compare_str_cstr(_l_section->value_types[_l_idx], "string")) {
                    string _temp;
                    init_str(&_temp);
                    if (!strcmp(_e.operator, "+")) {
                        add_cstr_to_str(&_temp, (char*)_l_section->values[_l_idx]);
                        add_cstr_to_str(&_temp, (char*)_r_section->values[_r_idx]);
                        printf("%s\n", str_to_cstr(&_temp));
                    }
                    else printf("this operator is not supported for this type");
                    free(_temp.data);
                }
                else {
                    double _temp = 0;
                    if (!strcmp(_e.operator, "+")) {
                        _temp += (intptr_t)_l_section->values[_l_idx];
                        _temp += (intptr_t)_r_section->values[_r_idx];
                        printf("%ld\n", (intptr_t)_temp);
                    }
                    else if (!strcmp(_e.operator, "-")) {
                        _temp += (intptr_t)_l_section->values[_l_idx];
                        _temp -= (intptr_t)_r_section->values[_r_idx];
                        printf("%ld\n", (intptr_t)_temp);
                    }
                    else if (!strcmp(_e.operator, "*")) {
                        _temp += (intptr_t)_l_section->values[_l_idx];
                        _temp *= (intptr_t)_r_section->values[_r_idx];
                        printf("%ld\n", (intptr_t)_temp);
                    }
                    else if (!strcmp(_e.operator, "/")) {
                        _temp += (intptr_t)_l_section->values[_l_idx];
                        _temp /= (intptr_t)_r_section->values[_r_idx];
                        printf("%f\n", _temp);
                    }
                }
            }
            else printf("expression parameters have different types");
        }
        free(_e.left_operand->key);
        free(_e.left_operand->value);
        free(_e.right_operand->key);
        free(_e.right_operand->value);
        free(_e.left_operand);
        free(_e.right_operand);
        free(_e.operator);
    }

    /*
    czyszczenie pamięci 🥶🥶
    */

    for (int i = 0; i < _i->len; i++) {
        free(_i->sections[i]->key->data);
        free(_i->sections[i]->key);
        for (int j = 0; j < _i->sections[i]->len; j++) {
            free(_i->sections[i]->value_names[j]->data);
            if (!compare_str_cstr(_i->sections[i]->value_types[j], "string"))
                free(_i->sections[i]->values[j]);
            free(_i->sections[i]->value_types[j]->data);
            free(_i->sections[i]->value_names[j]);
            free(_i->sections[i]->value_types[j]);            
        }
        free(_i->sections[i]->values);
        free(_i->sections[i]->value_names);
        free(_i->sections[i]->value_types);
        free(_i->sections[i]);
    }
    free(_i->sections);
}

int main(int argc, char** argv)
{
    ini_file i;
    init_ini_file(&i);
    parse_ini(&i, argv, argc);
}