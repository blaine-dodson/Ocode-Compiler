/*******************************************************************************
 *
 *	occ : The Omega Code Compiler
 *
 *	Copyright (c) 2016 Ammon Dodson
 *
 ******************************************************************************/


#include "compiler.h"


static void Initialize(yuck_t * arg_pt){
	uint sum;
	
	if (arg_pt->nargs > 1) puts("Too many arguments...Ignoring.");
	
	// set the global verbosity
	verbosity=arg_pt->dashv_flag;
	if(verbosity) printf("\
ARGUMENTS PASSED\n\
nargs             :\t%lu\n\
args              :\t%s\n\
dashv_flag        :\t%u\n\
dashD_arg         :\t%s\n\
dashd_flag        :\t%u\n\
dashp_flag        :\t%u\n\
dasha_flag        :\t%u\n\
x86_long_flag     :\t%u\n\
x86_protected_flag:\t%u\n\
arm_v7_flag       :\t%u\n\
arm_v8_flag       :\t%u\n\n" ,
			arg_pt->nargs      ,
			*arg_pt->args      ,
			arg_pt->dashv_flag ,
			arg_pt->dashD_arg  ,
			arg_pt->dashd_flag ,
			arg_pt->dashp_flag ,
			arg_pt->dasha_flag ,
			arg_pt->x86_long_flag     ,
			arg_pt->x86_protected_flag,
			arg_pt->arm_v7_flag       ,
			arg_pt->arm_v8_flag
		);
	
	// test for a target architecture
	sum = arg_pt->x86_long_flag + arg_pt->x86_protected_flag;
	sum += arg_pt->arm_v7_flag + arg_pt->arm_v8_flag;
	
	if(arg_pt->dasha_flag && sum != 1)
		crit_error("Must specify exactly one target architecture.");
	
	// initialize the symbol table
	symbols = DS_new(
		DS_bst,
		sizeof(struct sym),
		false,
		&cmp_sym,
		&cmp_sym_key
	);
	
	// initialize the intermediate code queues
	global_inst_q = DS_new(
		DS_list,
		sizeof(icmd),
		false,
		NULL,
		NULL
	);
	
	sub_inst_q = DS_new(
		DS_list,
		sizeof(icmd),
		false,
		NULL,
		NULL
	);
}


static void Parse(yuck_t * arg_pt){
	bool errors;
	
	// Set the infile
	if(arg_pt->nargs){
		yyin = fopen(*arg_pt->args, "r");
		if(!yyin) crit_error("No such file");
		printf("Reading from: %s\n", *arg_pt->args);
	}
	else puts("Reading from: stdin");
	
	// Set the debug file
	if (arg_pt->dashd_flag){
		char *dbgfile;
		
		dbgfile = (char*) malloc(strlen(*arg_pt->args)+strlen(default_dbg)+1);
		if(! dbgfile) crit_error("Out of Memory");
		
		dbgfile = strcpy(dbgfile, *arg_pt->args);
		dbgfile = strncat(dbgfile, default_dbg, strlen(default_dbg));
		debug_fd = fopen(dbgfile, "w");
		
		fprintf(debug_fd,"#Omnicode Intermediate File\n");
		
		free(dbgfile);
	}
	
	get_token(); // Initialize the lookahead token
	emit_lbl(add_name("_#GLOBAL_START"));
	
	errors=setjmp(anewline); // Save the program state for error recovery
	
	do {
		Statement(0);
	} while (token != T_EOF);
	
	// TODO: add return statments to the global_inst_q
	
	// Close the infile
	fclose(yyin);
	
	// close the debug file
	if (debug_fd){
		fputs("\n\n", debug_fd);
		Dump_symbols();
		fclose(debug_fd);
		
		puts("\nI Q DUMP\n");
		Dump_iq(global_inst_q);
		Dump_iq(sub_inst_q);
	}
	
	if(errors){
		puts("Errors were found. Exiting...");
		exit(EXIT_FAILURE);
	}
}


static void Generate_code(yuck_t * arg_pt, DS blk_q){
	
	// pexe
	if (arg_pt->dashp_flag){
		char *pexefile;
		
		pexefile = (char*) malloc(strlen(*arg_pt->args)+strlen(default_pexe)+1);
		if(! pexefile) crit_error("Out of Memory");
		
		pexefile = strcpy(pexefile, *arg_pt->args);
		pexefile = strncat(pexefile, default_pexe, strlen(default_pexe));
		
		if(verbosity) printf("pexefile is: %s\n", pexefile);
		
		pexe(pexefile, blk_q);
		
		free(pexefile);
	}
	
	// asm
	if (arg_pt->dasha_flag || !arg_pt->dashp_flag){
		char *asmfile;
		
		if (arg_pt->nargs){
			asmfile = (char*) malloc(strlen(*arg_pt->args)+strlen(default_asm)+1);
			if(! asmfile) crit_error("Out of Memory");
			asmfile = strcpy(asmfile, *arg_pt->args);
		}
		else{
			asmfile = (char*) malloc(strlen(default_out)+strlen(default_asm)+1);
			if(! asmfile) crit_error("Out of Memory");
			asmfile = strcpy(asmfile, default_out);
		}
		
		asmfile = strncat(asmfile, default_asm, strlen(default_asm));
		
		if(verbosity) printf("asmfile is: %s\n", asmfile);
		
		if(arg_pt->x86_long_flag) x86(asmfile, true, blk_q);
		else if(arg_pt->x86_protected_flag) x86(asmfile, false, blk_q);
		
		free(asmfile);
	}
}


static void Cleanup(yuck_t * arg_pt, DS structure){
	DS blk;

	if(verbosity) puts("Cleanup");
	yuck_free(arg_pt);
	DS_delete(symbols);
	DS_delete(global_inst_q);
	DS_delete(sub_inst_q);
	
	while(( blk = (DS) DS_first(structure) )) DS_delete(blk);
	DS_delete(structure);
	
	free(name_array);
}


int main (int argc, char** argv){
	yuck_t arg_pt[1];
	DS blk_q;
	
	yuck_parse(arg_pt, argc, argv);
	Initialize(arg_pt);
	Parse(arg_pt);
	
	blk_q = Optomize(global_inst_q, sub_inst_q);
	
	Generate_code(arg_pt, blk_q);
	
	Cleanup(arg_pt, blk_q);
	
	return EXIT_SUCCESS;
}


