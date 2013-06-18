def GenSwizzleDecl(string, type1, type2):
	for a in range(0, len(string)):
		print "\t" + type1 + " " + string[a] + "() const;"
	for a in range(0, len(string)):
		for b in range(0, len(string)):
			print "\tconst " + type2 + "2 " + string[a] + string[b] + "() const;"
	for a in range(0, len(string)):
		for b in range(0, len(string)):
			for c in range(0, len(string)):
				print "\tconst " + type2 + "3 " + string[a] + string[b] + string[c] + "() const;"
	for a in range(0, len(string)):
		for b in range(0, len(string)):
			for c in range(0, len(string)):
				for d in range(0, len(string)):
					print "\tconst " + type2 + "4 " + string[a] + string[b] + string[c] + string[d] + "() const;"

def GenSwizzleDef(string, type1, type2):
	for a in range(0, len(string)):
		print "inline " + type1 + " " + type2 + str(len(string)) + "::" + string[a] + "() const {return Swizzle<" + str(a) + ">();}"
	for a in range(0, len(string)):
		for b in range(0, len(string)):
			print "inline const " + type2 + "2 " + type2 + str(len(string)) + "::" + string[a] + string[b] + "() const {return Swizzle<" + str(a) + ", " + str(b) + ">();}"
	for a in range(0, len(string)):
		for b in range(0, len(string)):
			for c in range(0, len(string)):
				print "inline const " + type2 + "3 " + type2 + str(len(string)) + "::" + string[a] + string[b] + string[c] + "() const {return Swizzle<" + str(a) + ", " + str(b) + ", " + str(c) + ">();}"
	for a in range(0, len(string)):
		for b in range(0, len(string)):
			for c in range(0, len(string)):
				for d in range(0, len(string)):
					print "inline const " + type2 + "4 " + type2 + str(len(string)) + "::" + string[a] + string[b] + string[c] + string[d] + "() const {return Swizzle<" + str(a) + ", " + str(b) + ", " + str(c) + ", " + str(d) + ">();}"

GenSwizzleDecl("xy", "float", "Vector");
GenSwizzleDecl("xyz", "float", "Vector");
GenSwizzleDecl("xyzw", "float", "Vector");
GenSwizzleDecl("xy", "bool", "BVector");
GenSwizzleDecl("xyz", "bool", "BVector");
GenSwizzleDecl("xyzw", "bool", "BVector");

GenSwizzleDef("xy", "float", "Vector");
GenSwizzleDef("xyz", "float", "Vector");
GenSwizzleDef("xyzw", "float", "Vector");
GenSwizzleDef("xy", "bool", "BVector");
GenSwizzleDef("xyz", "bool", "BVector");
GenSwizzleDef("xyzw", "bool", "BVector");
