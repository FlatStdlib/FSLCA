#include "init.h"

public _asmblr init_assembler(string filename)
{
	_asmblr a;
	memzero(&a, sizeof(_asmblr));

	a.file = open_file(filename, 0, 0);
	if(!a.file)
		return (_asmblr){0};

	a.filename = str_dup(filename);
	if((a.filesize = file_content_size(a.file)) <= 0)
		fsl_panic("Unable to read file...!");

	a.content = allocate(0, a.filesize + 1);
	int bytes = file_read(a.file, a.content, a.filesize);
	if(bytes <= 0)
		fsl_panic("Unable to read file...!");

	file_close(a.file);
	a.ast = allocate(0, sizeof(fn_t));
	a.ast_count = 0;
	return a;
}


/* For The Language Parser */
public fn parse_file(_asmblr *a, bool struct_only, int cline)
{
	bool in_quotes = false, in_var = false, in_func = false, ignore = false;
	char word[1024], line[1024];
	i32 idx = 0, len = 0, lines = 0;
	for(int i = cline ? cline : 0; i < a->filesize; i++)
	{
		if(a->content[i] == '/' && a->content[i + 1] == '/') {
			while(a->content[i++] != '\n');
			i--;
			continue;
		} else if(a->content[i] == '/' && a->content[i + 1] == '*') {
			ignore = true;
			continue;
		} else if(ignore && a->content[i] == '*' && a->content[i + 1] == '/') {
			ignore = false;
			continue;
		}

		/* Ignore comments formatted like this */
		if(ignore) continue;

		if(validate_whitespace(a->content[i]))
		{
			if(a->content[i] == '\n') lines++;
			word[0] = '\0';
			idx = 0;
			continue;
		}

		/* Tokens */
		if(a->content[i] == ' ')
		{

			word[idx++] = '\0';
			_Datatype type;
			char ch = '\0';
			if((type = find_type(word)) != _DT_NULL)
			{

				char symbol[1024]; /* Symbol Name*/
				int arg_len = 0, n = i + 1;
				while(a->content[n] != ' ' && a->content[n] != '\n' && a->content[n] != '\r' && a->content[n] != '{' && a->content[n] != '[' && a->content[n] != '(' && a->content[n] != ';' && a->content[n] != '\t')
					symbol[arg_len++] = a->content[n++];

				symbol[arg_len] = '\0';

				if(!struct_only && str_cmp(word, (string)(__DATA_TYPES_INFO__[_DT_struct][1])))
				{
//					print("[Struct Found] Symbol Name: '"), print(symbol), print("'\n"); // Create array
					struct_t st = allocate(0, sizeof(_struct));
					process_struct(st, a->content, &n);
					while(a->content[n++] != '}');
					i = n;
				} else if((ch = get_next_symbol_only(a->content, n)) == '[' || ch == ';' || ch == '=') {
					print("[Variable Found] Symbol Name: '"), print(symbol), print("'\n"); // Get the fixed size
					var_t v = allocate(0, sizeof(_variable));
					memzero(v, sizeof(_variable));
					v->feature = n_variable;
					v->type = type;
					v->name = str_dup(symbol);
					process_variable(v, a->content, &n);

					a->ast[a->ast_count++] = (ptr)v;
					a->ast = reallocate(a->ast, sizeof(_variable) * (a->ast_count + 1));
					a->ast[a->ast_count] = NULL;
				} else if(!struct_only && get_next_symbol_only(a->content, n) == '(')
				{
					fn_t fnc = allocate(0, sizeof(_function));
					fnc->feature = n_function;
					fnc->type = type;
					fnc->name = str_dup(symbol);
					if(!process_function(fnc, a->content, &n))
						fsl_warning("Err to parse function!");

					print("[Function Found] Symbol Name: '"), print(symbol), print("'\n");
					_printf("Args: %d | Types: ", (ptr)&fnc->arg_count);
					for(int i = 0; i < fnc->arg_count; i++) {
						i == fnc->arg_count - 1 ? print(fnc->arg_types[i]) : _printf("%s ",fnc->arg_types[i]);
					}
					println(NULL);

					a->ast[a->ast_count++] = (ptr)fnc;
					a->ast = reallocate(a->ast, sizeof(fn_t) * (a->ast_count + 1));
					a->ast[a->ast_count] = NULL;
					i = n;
				} else {
					_printf("Line: %d | ", (ptr)&lines);
					fsl_warning("expected function or variable name!");
				}

				symbol[0] = '\0';
			}

			word[0] = '\0';
			idx = 0;
			continue;
		}

		if((in_quotes && a->content[i] == '"') || is_ascii_alpha(a->content[i]) || is_ascii_digit(a->content[i]))
		{
			if(idx >= 1023) { /* Doubt, It'll Reach Here */
				word[0] = '\0';
				idx = 0;
			}

			word[idx++] = a->content[i];
			// println(word);
		}

		// (*a->content)++;
	}

	pfree_array((array)a->lines);
	_pfree(a->filename);
	_pfree(a->content);
}

void bin_dump(_asmblr *a)
{
	for(int i = 0; i < a->ast_count; i++)
	{

	}
}
