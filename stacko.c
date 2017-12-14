#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct stack {
	void* end;
	struct stackValue* data;
};

struct internalFunction {
	const char* name;
	void (*fn)(struct stack*);
};

struct program {
	struct stackValue* start;
	void* end;
	struct stackValue* lastval;
	void* cheap;
};

struct stackValue {
	union {
		double n;
		const char* s;
		struct program p;
		void (*fn)(struct stack*);
	};
	enum {
		NUMBER, STRING, PROG, FN, END
	} type;
};


typedef struct stack stk;
typedef struct program stkProg;
typedef struct stackValue stkVal;
typedef struct internalFunction stkFun;

void interpret(stkProg, stk*);
void push(stkVal, stk*);
void printprog(stkProg);

stkFun* findsymbol(const char*);

stkProg parse(const char*);

void die(const char*);
char* findendqstr(const char*);
char* findendqcode(const char* sptr);

void pop(stk*);
void clear(stk*);
void length(stk*);
void print(stk*);
void printstack(stk*);
void halt(stk*);
void stackoif(stk*);
void stackoifelse(stk*);
void exec(stk*);
void dup(stk*);
void exch(stk*);
void find(stk*);
void roll(stk*);
void add(stk*);
void substract(stk*);
void equal(stk*);

stkProg progalloc(const char*);
stkVal* valalloc(stkProg*);
void* heapalloc(stkProg*, size_t);
stk stkalloc(size_t);

stkFun libstacko[] = {
	/*exposed name   internal symbol*/
	{ "pop",                     pop },
	{ "clear",                 clear },
	{ "length",               length },
	{ "print",                 print },
	{ "stack",            printstack },
	{ "halt",                   halt },
	{ "if",                 stackoif },
	{ "ifelse",         stackoifelse },
	{ "exec",                   exec },
	{ "dup",                     dup },
	{ "exch",                   exch },
	{ "find",                   find },
	{ "roll",                   roll },
	{ "+",                       add },
	{ "-",                 substract },
	{ "=",                     equal },
	{ NULL }
};

/* TODO: usar el field .fn */
#define FN(val)   (stkVal) { .type = FN,     .s = (val) }
#define NUM(val)  (stkVal) { .type = NUMBER, .n = (val) }
#define STR(val)  (stkVal) { .type = STRING, .s = (val) }
#define PROG(val) (stkVal) { .type = PROG,   .p = (val) }
#define END()     (stkVal) { .type = END }

int main(int argc, const char* argv[]) {
	stk stack;
	stkProg program;

	if(argc < 1) {
		puts("No programo given");
		return 0;
	}

	puts("-----------------");
	puts("     PROGRAM     ");
	puts("-----------------");
	puts(argv[1]);
	puts("");
	puts("-----------------");
	puts("     PARSING     ");
	puts("-----------------");
	program = parse(argv[1]);
	printprog(program);
	puts("");
	puts("-----------------");
	puts("PROGRAM EXECUTION");
	puts("-----------------");
	stack = stkalloc(2048);
	interpret(program, &stack);
	puts("");

	return 0;
}

void printprog(stkProg program) {
	stkVal* val = program.start;
	for(; val != program.lastval; val++)
	switch(val->type) {
	case NUMBER:
		printf("PUSH %f\n", val->n);
		break;
	case STRING:
		printf("PUSH \"%s\"\n", val->s);
		break;
	case PROG:
		puts("STARTPROG");
		printprog(val->p);
		puts("ENDPROG");
		break;
	case FN:
		printf("CALL %s\n", val->s);
		break;
	}
}

stkProg parse(const char* source) {
	const char* parsing = source;
	stkProg prog = progalloc(source);

	while(*parsing != '\0') {
		double num;
		char* parsed;
		/* Try to parse a number */
		num = strtod(parsing, &parsed);
		if(parsed != parsing) {
			parsing = parsed;
			*valalloc(&prog) = NUM(num);
			continue;
		}
		parsed = findendqstr(parsing);
		if(parsed != parsing) {
			size_t qstrsiz = parsed - parsing - 1;
			char* qstr = heapalloc(&prog, qstrsiz + 1);
			memcpy(qstr, parsing+1, qstrsiz);
			qstr[qstrsiz] = '\0';
			parsing = parsed + 1;
			*valalloc(&prog) = STR(qstr);
			continue;
		}
		parsed = findendqcode(parsing);
		if(parsed != parsing) {
			size_t progsiz = parsed - parsing - 1;
			char* progstr = heapalloc(&prog, progsiz + 1);
			memcpy(progstr, parsing+1, progsiz);
			progstr[progsiz] = '\0';
			parsing = parsed + 1;
			*valalloc(&prog) = PROG(parse(progstr));
			continue;
		}
		if(*parsing == ' '
		|| *parsing == '\n'
		|| *parsing == '\t') {
			parsing++;
			continue;
		}
		parsed = strpbrk(parsing, " \n\t");
		if(parsed != NULL) {
			size_t fnsiz = parsed - parsing;
			char* fnstr = heapalloc(&prog, fnsiz + 1);
			memcpy(fnstr, parsing, fnsiz);
			fnstr[fnsiz] = '\0';
			parsing = parsed;
			*valalloc(&prog) = FN(fnstr);
			continue;
		}
		/* FIXME: Asumo que esto es una funcion al final del fichero */
		{
			size_t fnsiz = strlen(parsing);
			char* fnstr = heapalloc(&prog, fnsiz + 1);
			memcpy(fnstr, parsing, fnsiz);
			fnstr[fnsiz] = '\0';
			parsing = parsing + fnsiz;
			*valalloc(&prog) = FN(fnstr);
		}
	}
	return prog;
}

char* findendqstr(const char* sptr) {
	if(*sptr == '"')
	for(sptr++; *sptr != '\0'; sptr++) {
		if(*sptr == '"') return (char*) sptr;
		if(*sptr == '\\')
			if(*(sptr+1) == '\0') break;
			else sptr++;
	}
	else return (char*) sptr;

	die("Erroro while parsing stringo, unexpectado finalo of main programo");
}

char* findendqcode(const char* sptr) {
	if(*sptr == '{')
	for(sptr++; *sptr != '\0'; sptr++) {
		if(*sptr == '{') sptr = findendqcode(sptr)+1;
		if(*sptr == '}') return (char*) sptr;
	}
	else return (char*) sptr;

	die("Erroro while parsing programo, unexpectado finalo of main programo");
}

void interpret(stkProg program, stk* stack) {	
	stkVal* val = program.start;
	for(; val != program.lastval; val++)
	switch(val->type) {
	case FN:
		findsymbol(val->s)->fn(stack);
		break;
	default:
		push(*val, stack);
		break;
	}
}

stkFun* findsymbol(const char* symbol) {
	stkFun* f = libstacko;
	for(; f->name != NULL; f++)
		if(strcmp(f->name, symbol) == 0) return f;
	die("Erroro while executing programo, function not found");
}

void push(stkVal value, stk* stack) {
	if(stack->data == stack->end) die("Erroro, stacko overflowo");
	else *(++stack->data) = value;
}

stkVal popval(stk* stack) {
	return *((stack->data)--);
}

double popnum(stk* stack) {
	stkVal v = popval(stack);
	if(v.type != NUMBER) die("Erroro, numbero expectedo");
	return v.n;
}

stkProg popprog(stk* stack) {
	stkVal v = popval(stack);
	if(v.type != PROG) die("Erroro, programo expectedo");
	return v.p;
}

/*
 * libstacko
 */

void length(stk* stack) {
	int i = 0;
	stkVal* val = stack->data;
	for(; val->type != END; i++, val--);
	push(NUM(i), stack);
}

void print(stk* stack) {
	stkVal* val = stack->data;
	pop(stack);

	switch(val->type) {
	case NUMBER:
		printf("%f\n", val->n);
		break;
	case STRING:
		puts(val->s);
		break;
	case PROG:
		puts("-- prog --");
		break;
	case FN:
		printf("-- fn: %s --", val->s);
		break;
	}
}

void printstack(stk* stack) {
	stkVal* v = stack->data;
	for(; v->type != END; v--)
	switch(v->type) {
	case NUMBER:
		printf("%f\n", v->n);
		break;
	case STRING:
		puts(v->s);
		break;
	case PROG:
		puts("-- prog --");
		break;
	case FN:
		printf("-- fn: %s --", v->s);
		break;
	}
	puts("-- end --");
}

void pop(stk* stack) {
	stack->data--;
}

void clear(stk* stack) {
	for(; stack->data->type != END; stack->data--);
}

void halt(stk* stack) {
	die("-- stacko finalo --");
}

void stackoif(stk* stack) {
	double cond = popnum(stack);
	if(cond) exec(stack);
}

void stackoifelse(stk* stack) {
	double cond = popnum(stack);
	stkProg onfalse = popprog(stack);
	stkProg ontrue = popprog(stack);
	interpret(cond ? ontrue : onfalse, stack);
}

void exec(stk* stack) {
	stkProg prog = popprog(stack);
	interpret(prog, stack);
}

void dup(stk* stack) {
	stkVal v = popval(stack);
	push(v, stack);
	push(v, stack);
}

void exch(stk* stack) {
	stkVal a = popval(stack);
	stkVal b = popval(stack);
	push(a, stack);
	push(b, stack);
}

void find(stk* stack) {
	int idx = (int) popnum(stack);
	/* TODO: chequear no salirse del stack */
	push(*(stack->data - idx), stack);
}

void roll(stk* stack) {
	int amount = (int) popnum(stack);
	int length = (int) popnum(stack);
	int i = 0;
	stkVal temp = *stack->data;

	amount = amount % length;

	if(amount < 0) amount += length;
	if(amount == 0) return;

	for(; i < length; i++)
		*(stack->data - i) = *(stack->data - (i + 1) % length);

	*(stack->data - length + 1) = temp;

	push(NUM(length), stack);
	push(NUM(amount - 1), stack);
	roll(stack);
}

void add(stk* stack) {
	double result = popnum(stack) + popnum(stack);
	push(NUM(result), stack);
}

void substract(stk* stack) {
	double b = popnum(stack);
	double a = popnum(stack);
	double result = a - b;
	push(NUM(result), stack);
}

void equal(stk* stack) {
	double a = popnum(stack);
	double b = popnum(stack);
	push(NUM(a==b), stack);
}

/*
 * Library things
 */

void die(const char* msg) {
	fputs(msg, stderr);
	fputc('\n', stderr);
	exit(-1);
}

stkProg progalloc(const char* program) {
	/* Allocates the maximum amount of memory a stacko
	 * program can need */
	size_t size = (strlen(program) / 2 + 2) * sizeof(stkVal);
	void* mem = malloc(size);
	return (stkProg) { .start = mem, .lastval = mem, .end = mem+size, .cheap = mem+size };
}

stkVal* valalloc(stkProg* program) {
	return (program->lastval)++;
}

void* heapalloc(stkProg* program, size_t size) {
	return program->cheap = program->cheap - size;
}

stk stkalloc(size_t values) {
	stk stack;
	size_t allocsiz = sizeof(stkVal) * values;
	stack.data = malloc(allocsiz);
	stack.end = stack.data + allocsiz;
	(*stack.data) = END();
	return stack;
}
