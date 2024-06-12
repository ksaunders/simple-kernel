#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAXSYM		100
#define MAXDEPTH	100

enum {
	TIDENT	= 127,
	TCONST,
	TWHILE,
	TEOF,
};

enum {
	OCONST	= 1,
	ODEREF,
	OASSIGN,
};

static int	peek, previous;
static int	diags, line = 1;
static int	n;
static char	name[MAXSYM];
static int	label = 1;
static int	ops[MAXDEPTH];
static int	*sp = ops;

#define push(op)	*sp++ = (op)
#define pop()		(*--sp)

static void	expr(void);

static int
xisspace(int c)
{
	return c == ' ' || c == '\t' || c == '\r';
}

static int
xisalpha(int c)
{
	return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

static int
xisdigit(int c)
{
	return c >= '0' && c <= '9';
}

static int
xisalnum(int c)
{
	return xisalpha(c) || xisdigit(c);
}

static int
xishex(int c)
{
	return xisdigit(c) || (c >= 'a' && c <= 'f') || (c >= 'A' && c <= 'F');
}

static void
diag(char *m)
{
	fprintf(stderr, "%s	%d\n", m, line);
	diags++;
	if (diags > 5)
		exit(diags);
}

static int
peekc(int op)
{
	int c = getchar();
	if(c == op)
		return 1;
	ungetc(c, stdin);
	return 0;
}

static int
lex(void)
{
	char *s = name, *e = s + MAXSYM - 1;

	for(;;){
		int c = getchar();
		if(c == EOF)
			return TEOF;
		if(c == '\n'){
			line++;
			continue;
		}
		if(xisspace(c))
			continue;
		switch(c) {
		case '(':
		case ')':
		case '{':
		case '}':
		case '*':
		case '=':
		case ';': return c;
		case '/':
			if(peekc('*')){
				for(;;){
					c = getchar();
					switch(c){
					case '\n':
						line++;
						break;
					case EOF:
						diag("lx");
						return TEOF;
					case '*':
						if(peekc('/'))	goto skip;
						break;
					}
				}
			}
			return c;
		case '\'':
			n = 0;
			while((c = getchar()) != '\''){
				if(c == EOF){
					diag("lx");
					return TEOF;
				}
				n = (n << 8) + c;
			}
			return TCONST;
		case '0':
			if(peekc('x') || peekc('X')){
				n = 0;
				while(xishex(c = getchar())){
					n *= 16;
					if(c >= '0' && c <= '9')
						n += (c - '0');
					else if(c >= 'a' && c <= 'f')
						n += (c - 'f');
					else
						n += (c - 'F');
				}
				ungetc(c, stdin);
				return TCONST;
			}
			break;
		}
		if(xisalpha(c)){
			while(xisalnum(c)){
				if(s < e)
					*s++ = c;
				c = getchar();
			}
			*s++ = 0;
			ungetc(c, stdin);
			if(strcmp(name, "while") == 0)
				return TWHILE;
			return TIDENT;
		}
		if(xisdigit(c)){
			n = 0;
			while(xisdigit(c)){
				n = n * 10 + (c - '0');
				c = getchar();
			}
			ungetc(c, stdin);
			return TCONST;
		}
		diag("lx");
skip:
	}
}

static void
advance(void)
{
	previous = peek;
	peek = lex();
}

static void
consume(int op)
{
	if(peek != op)
		diag("sx");
	advance();
}

static int
match(int op)
{
	if(peek == op){
		advance();
		return 1;
	}
	return 0;
}

static void
codegen(void)
{
	int a;
	int op;

	switch(op = pop()){
	case OCONST:
		a = pop();
		printf("	li	t0, %#x\n", a);
		printf("	sw	t0, -4(sp)\n");
		printf("	addi	sp, sp, -4\n");
		break;
	case ODEREF:
		codegen();
		break;
	case OASSIGN:
		codegen();
		codegen();
		printf("	lw	t0, 0(sp)\n");
		printf("	lw	t1, 4(sp)\n");
		printf("	addi	sp, sp, 8\n");
		printf("	sw	t1, 0(t0)\n");
		break;
	default:
		diag("cx");
	}
}

static void
primary(void)
{
	if(match(TCONST)){
		push(n);
		push(OCONST);
	}else if(match('(')){
		expr();
		consume(')');
	}else
		diag("ex");
}

static void
unary(void)
{
	if(match('*')){
		unary();
		push(ODEREF);
	}else
		primary();
}

static void
term(void)
{
	unary();
	while(match('=')){
		unary();
		push(OASSIGN);
	}
}

static void
expr(void)
{
	term();
}

static void
stmt(void)
{
	int loop;

	if(match(TWHILE)){
		consume('(');
		expr();
		printf("%d:", loop = label++);
		consume(')');
		codegen();
		stmt();
		printf("	lw	t1, 0(sp)\n");
		printf("	addi	sp, sp, 4\n");
		printf("	bne	t1, zero, %db\n", loop);
		return;
	}
	expr();
	consume(';');
	codegen();
}

int
main(int argc, char *argv[])
{
	for(;;){
		advance();
		if(peek == TEOF)
			return diags;
		consume(TIDENT);
		printf("	.global %s\n", name);
		printf("%s:\n", name);
		consume('(');
		consume(')');
		stmt();
		printf("	jr	ra\n");
	}
	return diags;
}
