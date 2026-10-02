#pragma once

// Result of a helper extracted from a larger method by
// .agents/research/extract-method.py. The caller turns it back into the same
// control transfer at the position where the extracted block used to be.
enum class ExtractedFlow
{
    Next,     // continue with the statement after the extracted block
    Continue, // continue the enclosing loop
    Break,    // break out of the enclosing loop or switch
    Return    // return from the calling method
};
