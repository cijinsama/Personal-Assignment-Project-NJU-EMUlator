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
#include <memory/paddr.h>

#define MAX_NUMBER_BUFFER 32
#define MAX_NUMBER_SINGAL 32
#define ERROR_MESSAGE_UNKNOWN -1
#define ERROR_MESSAGE_VALUE -2
#define ERROR_MESSAGE_OK -3
#define ERROR_MESSAGE_DEVIDE_ZERO -4
#define ERROR_MESSAGE_BACK_IS_GREATER -5
#define ERROR_MESSAGE_NUMBER_END -6
#define MAX_NUMBER_HEX 32

enum {
  TK_NOTYPE = 256, TK_NUMBER_NOEND = 257, TK_NUMBER_END = 258,TK_HEX = 259,TK_REG = 260, TK_EQ = 261, TK_UEQ = 262, DEREF = 263, NEGTIVE = 264, TK_AND = 265
  /* TODO: Add more token types */
};

static struct rule {
  const char *regex;
  int token_type;
} rules[] = {

  /* TODO: Add more rules.
   * Pay attention to the precedence level of different rules.
   */

  {" +", TK_NOTYPE},																			// spaces
  {"\\+", '+'},																						// plus
	{"\\-", '-'},																						// minus
	{"\\*", '*'},																						// times
	{"/", '/'},																							// divide
	{"\\(", '('},																						// quotes_left
	{"\\)", ')'},																						// quotes_right
	{"0x[0-9a-f]{1,32}", TK_HEX},								// hexadecimal-number  从{1,MAX_NUMBER_HEX}改成了+
	//{"[0-9]{32}(?=[0-9])", TK_NUMBER_NOEND},	// number0-9,the non end part
	//{"[0-9]{1,32}\\(?![0-9]\\)", TK_NUMBER_END},// number0-9,the end part_测试是否可以用32个
	{"[0-9]+", TK_NUMBER_END},															// number0-9
	{"\\$[a-zA-Z0-9\\$]+", TK_REG},														// reg_name
  {"==", TK_EQ},																					// equal
	{"!=", TK_UEQ},																					// unequal
	{"&&", TK_AND},																					// and
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
  int type;//这里改掉了=0
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
					case TK_EQ:
					case TK_UEQ:
					case TK_AND:
					case TK_NUMBER_NOEND:
					case TK_NUMBER_END:
						memcpy(tokens[nr_token].str, substr_start,substr_len);
						break;
					case TK_HEX:
						if (substr_len > MAX_NUMBER_HEX){
							printf("HEX number is larger than storage\n");
							return false;
						}
						else{
							memcpy(tokens[nr_token].str,substr_start+2,substr_len-2);
						}
						break;
					case TK_REG:
						if (*(substr_start+1) == '$') {
							memcpy(tokens[nr_token].str, substr_start+2,substr_len-1);
						}
						memcpy(tokens[nr_token].str, substr_start+1,substr_len-1);
						break;
					default:
						printf("Unknow expression\n");	
        }
				if (rules[i].token_type != TK_NOTYPE) {
					tokens[nr_token].type = rules[i].token_type;
					nr_token++;
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
#if 1==0
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
	if (back_pointer->type == '(' && front_pointer->type == ')'){
		int count = 0;
		int i = 0;
		if (back_pointer[i].type == '(') {
			count++;
		}
		for (i = 1; back_pointer + i <= front_pointer && count > 0; i++) {
			if (back_pointer[i].type == '(') {
				count++;
			}
			if (back_pointer[i].type == ')') {
				count--;
			}
		}
		if (count == 0 && (back_pointer+i) > front_pointer) { return true; }
	}
	return false;
}


uint32_t eval(Token* back_pointer, Token* front_pointer, int* error_message) {
	/*Error input*/
	if (back_pointer > front_pointer) {
		*error_message = ERROR_MESSAGE_BACK_IS_GREATER;
		fprintf(stderr, "back_pointer is larger than front_pointer\nin line : %d and file : %s\n",__LINE__, __FILE__);
		return 1;
	}
	else if (back_pointer->type == TK_NUMBER_NOEND) {
		eval(back_pointer+1,front_pointer,error_message);
	}
	/*when expr is number and is the end of it*/
	else if (back_pointer == front_pointer) {
		if (back_pointer->type == TK_HEX) {
			uint32_t single_number = 0;
			char curchar;
			for (int i=0; i<strlen(back_pointer->str); i++){
				curchar = back_pointer->str[i];	
				if (curchar <= 9 && curchar >= 0) single_number = single_number * 10 + curchar - '0';
				else single_number = single_number * 10 + curchar - 'a' + 10;
			}
			return single_number;	
		}
		else if (back_pointer->type == TK_REG) {
			bool success;
			uint32_t single_number = isa_reg_str2val(back_pointer->str, &success);
			if (success) {
				return single_number;
			}
			*error_message = ERROR_MESSAGE_NUMBER_END;	
			return 1;
		}
		else if (back_pointer->type != TK_NUMBER_END) {
			*error_message = ERROR_MESSAGE_VALUE;
			fprintf(stderr, "pointer is supposed to point at a number, but got token type %d\nin line : %d and file : %s\n", back_pointer->type,__LINE__, __FILE__);
			return 1;
		}
		else {/*now, the back_pointer to a TK_NUMBER_END, back search for the whole number*/
			uint32_t single_number = 0;
			/*backforward search for the whole number string*/
			while (back_pointer->type == TK_NUMBER_NOEND || back_pointer->type == TK_NUMBER_END) { back_pointer--; }
			/*deal with the nonend part*/
			while ((++back_pointer)->type == TK_NUMBER_NOEND) {
				 for (int i=0; i<MAX_NUMBER_BUFFER; i++) {
					single_number = single_number*10 + back_pointer->str[i] - '0';
				 }   
			}
			/*deal with the end part*/
			for (int i=0; back_pointer->str[i]!=0 && i< MAX_NUMBER_SINGAL; i++){
				single_number = single_number*10 + back_pointer->str[i] - '0';
			}
			return single_number;
		}
	}
	else if (check_parentheses(back_pointer, front_pointer) == true) {
	/* The expression is surrounded by a matched pair of parentheses.
	* If that is the case, just throw away the parentheses.
	*/
		return eval(back_pointer + 1, front_pointer - 1, error_message);
	}
	else {
		/*find the main operator, which means the last operator*/
		/*由于main_op是最后一个运算的符号，因此运算优先级越低成为main_op的优先级越高,即本来优先级高的应该被放在前面，因为最前面的最先被替换掉*/
		Token* main_op = NULL;
		int count = 0;	//用 count 作为括号匹配栈
		uint32_t val1 = 0,val2 = 0;
		for (int i=0; back_pointer+i <= front_pointer; i++) {
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
			else if (back_pointer[i].type == DEREF || back_pointer[i].type == NEGTIVE) {
				if (main_op == NULL || main_op->type == DEREF || main_op->type == NEGTIVE) {
					main_op = back_pointer + i;
				}
			}
			else if (back_pointer[i].type == '*' || back_pointer[i].type == '/') {
				if (main_op == NULL || main_op->type == DEREF || main_op->type == NEGTIVE || main_op->type == '*' || main_op->type == '/') {
					main_op = back_pointer + i;
				}
			}
			else if (back_pointer[i].type == '+' || back_pointer[i].type == '-') {
				if (main_op == NULL || main_op->type == DEREF || main_op->type == NEGTIVE || main_op->type == '*' || main_op->type == '/' || main_op->type == '+' || main_op->type == '-') {
					main_op = back_pointer + i;
				}
			}
			else if (back_pointer[i].type == TK_EQ || back_pointer[i].type == TK_UEQ) {
				if (main_op == NULL || main_op->type == DEREF || main_op->type == NEGTIVE || main_op->type == '*' || main_op->type == '/' || main_op->type == '+' || main_op->type == '-' || main_op->type == TK_EQ || main_op->type == TK_UEQ) {
					main_op = back_pointer + i;
				}
			}
			else if (back_pointer[i].type == TK_AND) {
				if (main_op == NULL || main_op->type == DEREF || main_op->type == NEGTIVE || main_op->type == '*' || main_op->type == '/' || main_op->type == '+' || main_op->type == '-' || main_op->type == TK_EQ || main_op->type == TK_UEQ || main_op->type == TK_AND) {
					main_op = back_pointer + i;
				}
			}
		}
		if (main_op == NULL) {
			Assert(false,"didn't find the main_op");
		}
		/*use the main operator to eval*/
		if (main_op->type != DEREF && main_op->type != NEGTIVE){
			val1 = eval(back_pointer, main_op - 1, error_message);
			val2 = eval(main_op + 1, front_pointer, error_message);
		}
		else {
			val2 = eval(main_op + 1, front_pointer, error_message);
		}
		//printf("use the main operator %d : %s\n",main_op->type,main_op->str);/*2333333*/
		//printf("use the val1 %d, val2 %d\n",val1,val2);/*2333333*/
		//printf("use the back_pointer %s, front_pointer %s\n",back_pointer->str,front_pointer->str);/*2333333*/
		if (*error_message != ERROR_MESSAGE_OK && *error_message != 0) { return 1; }
		switch (main_op->type) {
			case '+': return val1 + val2;
			case '-': return val1 - val2;
			case '*': return val1 * val2;
			case '/': {
				if (val2 == 0) {
					*error_message = ERROR_MESSAGE_DEVIDE_ZERO;
					fprintf(stderr, "Devide zero \nin line : %d and file : %s\n", __LINE__, __FILE__);
					return 1;
				}
				return val1 / val2;
			}
			case TK_EQ: return val1 == val2;
			case TK_UEQ: return val1 != val2;
			case TK_AND: return val1 && val2;
			case DEREF: return paddr_read(val2,4);
			case NEGTIVE: return -val2;
			default: {
				*error_message = ERROR_MESSAGE_UNKNOWN;
				fprintf(stderr, "Unkown error\nin line : %d and file : %s\n",__LINE__, __FILE__);
				return 1;
			}
		}
	}
	return 1;
}


word_t expr(char *e, bool *success) {
  if (!make_token(e)) {
    *success = false;
    return 0;
   }
	int error_message = 0;
	uint32_t ans = 0;
  
	/*处理*与-的二异性*/
	for (int i = 0; i < nr_token; i ++) {
		if (tokens[i].type == '-' && (i == 0 || (tokens[i - 1].type != ')' && tokens[i - 1].type != TK_NUMBER_END && tokens[i - 1].type != TK_NUMBER_NOEND && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_REG ) ) ) {
			tokens[i].type = NEGTIVE;
		}
		if (tokens[i].type == '*' && (i == 0 || (tokens[i - 1].type != ')' && tokens[i - 1].type != TK_NUMBER_END && tokens[i - 1].type != TK_NUMBER_NOEND && tokens[i - 1].type != TK_HEX && tokens[i - 1].type != TK_REG ) ) ) {
			tokens[i].type = DEREF;
		}
	}
	
	/* TODO: Insert codes to evaluate the expression. */		
	ans = eval(tokens, tokens+nr_token-1, &error_message);
	if (error_message == 0) {
		*success = true;
		return ans;
	}
	*success = false;
	printf("error_msg : %d\n", error_message);
	return 0;
}
