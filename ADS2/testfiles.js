const PostfixCalculator = require('./calculator'); // Adjust path as needed

/**
 * Test runner utility
 */
class TestRunner {
    constructor() {
        this.tests = [];
        this.passed = 0;
        this.failed = 0;
    }

    test(name, testFn) {
        this.tests.push({ name, testFn });
    }

    async run() {
        console.log('🧪 Running Postfix Calculator Tests\n');
        
        for (const { name, testFn } of this.tests) {
            try {
                await testFn();
                console.log(`✅ ${name}`);
                this.passed++;
            } catch (error) {
                console.log(`❌ ${name}`);
                console.log(`   Error: ${error.message}`);
                this.failed++;
            }
        }

        console.log(`\n📊 Test Results: ${this.passed} passed, ${this.failed} failed`);
        
        if (this.failed > 0) {
            process.exit(1);
        }
    }
}

/**
 * Test utilities
 */
function assertEquals(actual, expected, message = '') {
    if (actual !== expected) {
        throw new Error(`Expected ${expected}, got ${actual}. ${message}`);
    }
}

function assertArrayEquals(actual, expected, message = '') {
    if (JSON.stringify(actual) !== JSON.stringify(expected)) {
        throw new Error(`Expected [${expected.join(', ')}], got [${actual.join(', ')}]. ${message}`);
    }
}

function assertThrows(fn, expectedMessage = '') {
    try {
        fn();
        throw new Error(`Expected function to throw, but it didn't`);
    } catch (error) {
        if (expectedMessage && !error.message.includes(expectedMessage)) {
            throw new Error(`Expected error message to contain "${expectedMessage}", got "${error.message}"`);
        }
    }
}

// Create test runner
const runner = new TestRunner();

// ========== BASIC ARITHMETIC TESTS ==========

runner.test('Basic addition', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('3 4 +');
    assertEquals(calc.getTopResult(), 7);
    assertArrayEquals(calc.stack.toArray(), [7]);
});

runner.test('Basic subtraction', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('10 3 -');
    assertEquals(calc.getTopResult(), 7);
});

runner.test('Basic multiplication', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('5 6 *');
    assertEquals(calc.getTopResult(), 30);
});

runner.test('Basic division', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('15 3 /');
    assertEquals(calc.getTopResult(), 5);
});

runner.test('Division by zero throws error', () => {
    const calc = new PostfixCalculator();
    assertThrows(() => calc.evaluate('5 0 /'), 'Division by zero');
});

runner.test('Power operation', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('2 3 ^');
    assertEquals(calc.getTopResult(), 8);
});

runner.test('Modulo operation', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('7 3 %');
    assertEquals(calc.getTopResult(), 1);
});

runner.test('Modulo with mod keyword', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('7 3 mod');
    assertEquals(calc.getTopResult(), 1);
});

// ========== UNARY OPERATOR TESTS ==========

runner.test('Square root', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('25 sqrt');
    assertEquals(calc.getTopResult(), 5);
});

runner.test('Square root of negative throws error', () => {
    const calc = new PostfixCalculator();
    assertThrows(() => calc.evaluate('-4 sqrt'), 'Square root of negative');
});

runner.test('Factorial', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('5 !');
    assertEquals(calc.getTopResult(), 120);
});

runner.test('Factorial of 0', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('0 !');
    assertEquals(calc.getTopResult(), 1);
});

runner.test('Factorial of negative throws error', () => {
    const calc = new PostfixCalculator();
    assertThrows(() => calc.evaluate('-1 !'), 'non-negative integers');
});

runner.test('Factorial of decimal throws error', () => {
    const calc = new PostfixCalculator();
    assertThrows(() => calc.evaluate('3.5 !'), 'non-negative integers');
});

runner.test('Negation', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('5 neg');
    assertEquals(calc.getTopResult(), -5);
});

runner.test('Logarithm base 10', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('100 log');
    assertEquals(calc.getTopResult(), 2);
});

runner.test('Natural logarithm', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('2.718281828459045 ln');
    assertEquals(Math.round(calc.getTopResult() * 1000) / 1000, 1); // Round to handle floating point
});

runner.test('Trigonometric functions', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('0 sin');
    assertEquals(Math.round(calc.getTopResult() * 1000) / 1000, 0);
    
    calc.stack.clear();
    calc.evaluate('0 cos');
    assertEquals(calc.getTopResult(), 1);
});

// ========== SCIENTIFIC NOTATION TESTS ==========

runner.test('Scientific notation - basic', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('1e3');
    assertEquals(calc.getTopResult(), 1000);
});

runner.test('Scientific notation - decimal', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('1.5e2');
    assertEquals(calc.getTopResult(), 150);
});

runner.test('Scientific notation - negative exponent', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('1e-3');
    assertEquals(calc.getTopResult(), 0.001);
});

runner.test('Scientific notation - uppercase E', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('2E4');
    assertEquals(calc.getTopResult(), 20000);
});

runner.test('Scientific notation - negative number', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('-1.5e2');
    assertEquals(calc.getTopResult(), -150);
});

// ========== VARIABLE ASSIGNMENT TESTS ==========

runner.test('Variable assignment is silent', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('A 5 =');
    assertEquals(calc.getTopResult(), null, 'Assignment should be silent');
    assertArrayEquals(calc.stack.toArray(), [], 'Stack should be empty after assignment');
});

runner.test('Variable assignment and retrieval', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('A 10 =');
    calc.evaluate('A');
    assertEquals(calc.getTopResult(), 10);
});

runner.test('Variable in expression', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('A 5 =');
    calc.evaluate('A 3 +');
    assertEquals(calc.getTopResult(), 8);
});

runner.test('Multiple variable assignment', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('A 5 =');
    assertEquals(calc.getTopResult(), null);
    calc.evaluate('B 10 =');
    assertEquals(calc.getTopResult(), null);
    calc.evaluate('A B *');
    assertEquals(calc.getTopResult(), 50);
});

runner.test('Variable reassignment', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('A 5 =');
    calc.evaluate('A 10 =');
    calc.evaluate('A');
    assertEquals(calc.getTopResult(), 10);
});

runner.test('Assign variable to variable', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('A 5 =');
    calc.evaluate('B A =');
    calc.evaluate('B');
    assertEquals(calc.getTopResult(), 5);
});

runner.test('Invalid variable name throws error', () => {
    const calc = new PostfixCalculator();
    assertThrows(() => calc.evaluate('abc 5 ='), 'Invalid token');
});

// ========== CONSTANTS TESTS ==========

runner.test('Pi constant', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('pi');
    assertEquals(calc.getTopResult(), Math.PI);
});

runner.test('E constant', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('e');
    assertEquals(calc.getTopResult(), Math.E);
});

runner.test('Pi in calculation', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('pi 2 *');
    assertEquals(calc.getTopResult(), 2 * Math.PI);
});

runner.test('Assign constant to variable', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('H pi =');
    assertEquals(calc.getTopResult(), null, 'Assignment should be silent');
    calc.evaluate('H');
    assertEquals(calc.getTopResult(), Math.PI);
});

// ========== COMPLEX EXPRESSION TESTS ==========

runner.test('Complex expression with multiple operations', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('3 4 + 2 * 5 -');
    assertEquals(calc.getTopResult(), 9); // (3+4)*2-5 = 14-5 = 9
});

runner.test('Complex expression with variables', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('A 3 =');
    calc.evaluate('B 4 =');
    calc.evaluate('A B + 2 *');
    assertEquals(calc.getTopResult(), 14); // (3+4)*2 = 14
});

runner.test('Nested calculations', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('2 3 ^'); // 8
    calc.evaluate('3 !'); // 6
    calc.evaluate('+'); // 8 + 6 = 14
    assertEquals(calc.getTopResult(), 14);
});

// ========== STACK MANAGEMENT TESTS ==========

runner.test('Empty stack returns null', () => {
    const calc = new PostfixCalculator();
    assertEquals(calc.getTopResult(), null);
});

runner.test('Stack with multiple values', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('1 2 3');
    assertArrayEquals(calc.stack.toArray(), [1, 2, 3]);
    assertEquals(calc.getTopResult(), 3);
});

runner.test('Stack after operation', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('1 2 3 +');
    assertArrayEquals(calc.stack.toArray(), [1, 5]);
    assertEquals(calc.getTopResult(), 5);
});

runner.test('Clear stack', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('1 2 3');
    calc.reset();
    assertArrayEquals(calc.stack.toArray(), []);
    assertEquals(calc.getTopResult(), null);
});

// ========== ERROR HANDLING TESTS ==========

runner.test('Insufficient operands for binary operator', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('5');
    assertThrows(() => calc.evaluate('+'), 'Insufficient operands');
});

runner.test('Insufficient operands for unary operator', () => {
    const calc = new PostfixCalculator();
    assertThrows(() => calc.evaluate('sqrt'), 'Insufficient operands');
});

runner.test('Invalid token', () => {
    const calc = new PostfixCalculator();
    assertThrows(() => calc.evaluate('xyz'), 'Invalid token');
});

runner.test('Assignment without enough operands', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('A');
    assertThrows(() => calc.evaluate('='), 'Assignment requires');
});

runner.test('Empty expression', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('');
    assertEquals(calc.getTopResult(), null);
});

runner.test('Whitespace-only expression', () => {
    const calc = new PostfixCalculator();
    calc.evaluate('   ');
    assertEquals(calc.getTopResult(), null);
});

// ========== SYMBOL TABLE TESTS ==========

runner.test('Symbol table operations', () => {
    const calc = new PostfixCalculator();
    
    // Insert
    calc.evaluate('A 10 =');
    assertEquals(calc.symbolTable.has('A'), true);
    assertEquals(calc.symbolTable.search('A'), 10);
    
    // Update
    calc.evaluate('A 20 =');
    assertEquals(calc.symbolTable.search('A'), 20);
    
    // Delete
    assertEquals(calc.deleteVariable('A'), true);
    assertEquals(calc.symbolTable.has('A'), false);
    assertEquals(calc.deleteVariable('A'), false); // Already deleted
});

runner.test('Symbol table size', () => {
    const calc = new PostfixCalculator();
    assertEquals(calc.symbolTable.getSize(), 0);
    
    calc.evaluate('A 1 =');
    assertEquals(calc.symbolTable.getSize(), 1);
    
    calc.evaluate('B 2 =');
    assertEquals(calc.symbolTable.getSize(), 2);
    
    calc.deleteVariable('A');
    assertEquals(calc.symbolTable.getSize(), 1);
});

// ========== INTEGRATION TESTS ==========

runner.test('Scientific calculation scenario', () => {
    const calc = new PostfixCalculator();
    
    // Calculate area of circle: π * r²
    calc.evaluate('R 2.5 =');
    calc.evaluate('R 2 ^'); // r²
    calc.evaluate('pi *'); // π * r²
    
    const expectedArea = Math.PI * 2.5 * 2.5;
    assertEquals(Math.abs(calc.getTopResult() - expectedArea) < 0.000001, true);
});

// Run all tests
runner.run().catch(console.error);