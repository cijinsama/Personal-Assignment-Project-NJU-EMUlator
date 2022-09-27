/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include <isa.h>

/* We use the POSIX regex functions to process regular expressions.
 * Type 'man regex' for more information about POSIX regex functions.
 */
#include <regex.h>
#include <math.h>

#define MAX_NUMBER_BUFFER 32
#define MAX_NUMBER_SINGAL 32
#define ERROR_MESSAGE_UNKNOWN -1
#define ERROR_MESSAGE_VALUE -2

enum {
  TK_NOTYPE = 256, TK_EQ, TK_NUMBER_NOEND = 257, TK_NUMBER_END = 258,

  /* TODO: Add more token types */

};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},    // spaces
  {"\\+", '+'},         // plus
  {"==", TK_EQ},        // equal
	{"\\-", '-'},         // minus
	{"\\*", '*'},					// times
	{"/", '/'},						// divide
	{"\\(", '('},					// quotes_left
	{"\\)", ')'},					// quotes_right
	{"[0-9]{MAX_NUMBER_BUFFER}(?:[0-9])", TK_NUMBER_NOEND},	// number0-9,the non end part
	{"[0-9]{1,MAX_NUMBER_SINGAL}(?:[^0-9])", TK_NUMBER_END}															// number0-9,the end part_测试是否可以用32个
};

#define NR_REGEX ARRLEN(rules)

static regex_t re[NR_REGEX] = {};

/* Rules are used for many times.
 * Therefore we compile them only once before any usage.
 */
void init_regex() {
  int i;
  char error_msg[128];
  int ret;

  for (i = 0; i < NR_REGEX; i ++) {
    ret = regcomp(&re[i], rules[i].regex, REG_EXTENDED);
    if (ret != 0) {
      regerror(ret, &re[i], error_msg, 128);
      panic("regex compilation failed: %s\n%s", error_msg, rules[i].regex);
    }
  }
}

typedef struct token {
  int type = 0;
  char str[32];
} Token;

static Token tokens[32] __attribute__((used)) = {};
static int nr_token __attribute__((used))  = 0;

static bool make_token(char *e) {
  int position = 0;
  int i;
  regmatch_t pmatch;

  nr_token = 0;

  while (e[position] != '\0') {
    /* Try all rules one by one. */
    for (i = 0; i < NR_REGEX; i ++) {
      if (regexec(&re[i], e + position, 1, &pmatch, 0) == 0 && pmatch.rm_so == 0) {
        char *substr_start = e + position;
        int substr_len = pmatch.rm_eo;

        Log("match rules[%d] = \"%s\" at position %d with len %d: %.*s",
            i, rules[i].regex, position, substr_len, substr_len, substr_start);

        position += substr_len;

        /* TODO: Now a new token is recognized with rules[i]. Add codes
         * to record the token in the array `tokens'. For certain types
         * of tokens, some extra actions should be performed.
         */

        switch (rules[i].token_type) {
					case TK_NOTYPE: 
						break;
					case '+':
					case '-':
					case '*':
					case '/':
					case '(':
					case ')':
						tokens[nr_token].type = rules->token_type;
						tokens[nr_token].str[0] =(char)rules->token_type;
						nr_token++;
						break;
					case TK_NUMBER_NOEND:
						tokens[nr_token].type = rules->token_type;
					  memcpy(tokens[nr_token].str,substr_start,MAX_NUMBER_BUFFER);
						break;
					case TK_NUMBER_END:
						tokens[nr_token].type = rules->token_type;
					  memcpy(tokens[nr_token].str,substr_start,MAX_NUMBER_SINGAL);
						break;
          default:
						printf("Unknow expression\n");	
        }
        break;
      }
    }

    if (i == NR_REGEX) {
      printf("no match at position %d\n%s\n%*.s^\n", position, e, position, "");
      return false;
    }
  }

  return true;
}
#if 1==1
bool examin_if_make_token(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return false;
   }
  /* TODO: Insert codes to evaluate the expression. */
	char new_e[10000];
	int position_new = 0;
	int position = 0;
	while (tokens[position].type != 0) {
		/*如果被填满了，就全部复制过去，要不然用strlen获得储存的字符串长度，并复制过去*/
		if (tokens[position].str[32] != 0) {
			memcpy(&new_e[position_new], tokens[position].str,32);
		}
		else {
			memcpy(&new_e[position_new], tokens[position].str,strlen(tokens[position].str)-1);
		}
	}
	int ret = memcmp(new_e, e, strlen(e));	
	if (ret == 0) return true;
  return false;
}
#endif

bool check_parentheses(Token* back_pointer, Token* front_pointer) {
	if (back_pointer.type == '(' && back_pointer.type == ')'){
		int count = 0;
		for (int i=0; back_pointer+i <= front_pointer && count >= 0; i++) {
			if (back_pointer[i].type == '(') {
				count++;
			}
			if (back_pointer[i].type == ')') {
				count--;
			}
		}
		if (count == 0) { return true; }
	}
	return false;
}


uint32_t eval(Token* back_pointer, Token* front_pointer, int* error_message) {
	/*Error input*/
	if (back_pointer > front_pointer) {
		*error_message = ERROR_MESSAGE_UNKNOWN;
		fprintf(stderr, "back_pointer is larger than front_pointer\nin line : %d and file : %s\n",__LINE__, __FILE__);
		return 1;
	}
	else if (back_pointer.type == TK_NUMBER_NOEND) {
		eval(back_pointer+1,front_pointer,error_message);
	}
	/*when expr is number and is the end of it*/
	else if (back_pointer == front_pointer) {
		if (back_pointer.type != TK_NUMBER_END) {
			*error_message = ERROR_MESSAGE_VALUE;
			fprintf(stderr, "pointer is supposed to point at a number, but got token type %d\nin line : %d and file : %s\n", back_pointer.type,__LINE__, __FILE__);
			return 1;
		}
		uint32_t single_number = 0;
		/*backforward search for the whole number string*/
		while (back_pointer.type == TK_NUMBER_NOEND || back_pointer.type == TK_NUMBER_END) { back_pointer--; }
		/*deal with the nonend part*/
		while ((++back_pointer).type == TK_NUMBER_NOEND) {
			 for (int i=0; i<MAX_NUMBER_BUFFER; i++) {
				single_number = single_number*10 + back_pointer.str[i] - '0';
			 }   
		}
		/*deal with the end part*/
		for (int i=0; back_pointer.str[i]!=0 && i< MAX_NUMBER_SINGAL; i++){
			single_number = single_number*10 + back_pointer.str[i] - '0';
		}
		return single_number;
	}
	else if (check_parentheses(back_pointer, front_pointer) == true) {
	/* The expression is surrounded by a matched pair of parentheses.
	* If that is the case, just throw away the parentheses.
	*/
		return eval(back_pointer + 1, front_pointer - 1);
	}
	else {
		/*find the main operator*/
		Token* main_op = back_pointer;
		int count = 0;	//用 count 作为括号匹配栈
		for (int i=0; main_op+i <= front_pointer; i++) {
			if (count < 0) {
				*error_message = ERROR_MESSAGE_UNKNOWN;
				fprintf(stderr, "Unkown error\nin line : %d and file : %s\n",__LINE__, __FILE__);
				return 1;	
			}
			else if (count > 0) {
				if (back_pointer[i].type == '(') {count++; continue; }
				else if (back_pointer[i].type == ')') {count--; continue; }
			}
			else if (back_pointer[i].type == '(') { count++; continue; }
			else if (back_pointer[i].type == '*' || back_pointer[i].type == '/') {
				main_op = back_pointer + i;	
			}
			else if (back_pointer[i].type == '+' || back_pointer[i].type == '-') {
				if (main_op.type == '*' || back_pointer.type == '/') {
					continue;
				}
				else {
					main_op = back_pointer + i;
				}
			}
		}
		/*use the main operator to eval*/
		val1 = eval(p, main_op - 1, error_message);
		val2 = eval(main_op + 1, q, error_message);
		if (*error_message != ERROR_MESSAGE_OK) { return 1; }
		switch (op_type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 - val2;
			case '/': {
				if (val2 == 0) {
					*error_message = ERROR_MESSAGE_DEVIDE_ZERO;
					fprintf(stderr, "Devide zero \nin line : %d and file : %s\n", back_pointer.type,__LINE__, __FILE__);
					return 1;
				}
				return val1 / val2;
			}
			default: {
				*error_message = ERROR_MESSAGE_UNKNOWN;
				fprintf(stderr, "Unkown error\nin line : %d and file : %s\n",__LINE__, __FILE__);
				return 1;
			}
		}
	}
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
   }
	int error_message = 0;
	uint32_t ans = 0;
	eval()
  /* TODO: Insert codes to evaluate the expression. */		
	ans = eval(tokens, tokens+nr_token, error_message);
	if (error_message == 0) {
		*success = true;
		return ans;
	}
	*success = false;
	return 0;
}
