package ast

import (
	"bytes"
	"fmt"
)

// A loop statement that iterates over individual events.
type EventLoop struct {
	queryElementImpl
	statements Statements
}

// Creates a new event loop.
func NewEventLoop() *EventLoop {
	return &EventLoop{}
}

// Retrieves the function name used during codegen.
func (l *EventLoop) FunctionName(init bool) string {
	if init {
		return fmt.Sprintf("i%d", l.ElementId())
	}
	return fmt.Sprintf("a%d", l.ElementId())
}

// Retrieves the merge function name used during codegen.
func (l *EventLoop) MergeFunctionName() string {
	return ""
}

// Returns the statements executed for each loop iteration.
func (l *EventLoop) Statements() Statements {
	return l.statements
}

// Sets the loop's statements.
func (l *EventLoop) SetStatements(statements Statements) {
	for _, s := range l.statements {
		s.SetParent(nil)
	}
	l.statements = statements
	for _, s := range l.statements {
		s.SetParent(l)
	}
}

//--------------------------------------
// Code Generation
//--------------------------------------

// Generates Lua code for the loop.
func (l *EventLoop) CodegenAggregateFunction(init bool) (string, error) {
	buffer := new(bytes.Buffer)

	// Generate child statement functions.
	str, err := l.statements.CodegenAggregateFunctions(init)
	if err != nil {
		return "", err
	}
	buffer.WriteString(str)

	fmt.Fprintf(buffer, "%s\n", lineStartRegex.ReplaceAllString(l.String(), "-- "))
	fmt.Fprintf(buffer, "function %s(cursor, data)\n", l.FunctionName(init))
	fmt.Fprintln(buffer, "  repeat")
	fmt.Fprintln(buffer, "  if cursor.event:eof() or (cursor.max_timestamp > 0 and cursor.event:timestamp() >= cursor.max_timestamp) then return end")
	for _, statement := range l.statements {
		fmt.Fprintf(buffer, "    %s(cursor, data)\n", statement.FunctionName(init))
	}
	fmt.Fprintln(buffer, "  until not cursor:next()")
	fmt.Fprintln(buffer, "end")

	return buffer.String(), nil
}

// Generates Lua code for the query.
func (l *EventLoop) CodegenMergeFunction(fields map[string]interface{}) (string, error) {
	return l.statements.CodegenMergeFunctions(fields)
}

// Converts factorized fields back to their original strings.
func (l *EventLoop) Defactorize(data interface{}) error {
	return l.statements.Defactorize(data)
}

func (l *EventLoop) Finalize(data interface{}) error {
	return l.statements.Finalize(data)
}

func (l *EventLoop) RequiresInitialization() bool {
	return l.statements.RequiresInitialization()
}

//--------------------------------------
// Utility
//--------------------------------------

// Returns a list of variable references within this condition.
func (l *EventLoop) VarRefs() []*VarRef {
	return l.statements.VarRefs()
}

// Returns a list of variables declared within this statement.
func (l *EventLoop) Variables() []*Variable {
	return l.statements.Variables()
}

// Converts the loop to a string-based representation.
func (l *EventLoop) String() string {
	str := "FOR EACH EVENT\n"
	str += lineStartRegex.ReplaceAllString(l.statements.String(), "  ") + "\n"
	str += "END"
	return str
}
