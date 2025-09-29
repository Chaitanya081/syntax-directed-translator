# streamlit_app/app.py
import streamlit as st
import ast, operator as op
import shutil, subprocess, sys

st.set_page_config(page_title="Expr Eval", layout="centered")
st.title("Syntax-Directed Translator — Arithmetic Evaluator")

st.write("Enter an arithmetic expression. The deployed app evaluates using Python (safe evaluator). If you run locally and have a compiled C binary `expr_eval`, the app can call it.")

expr = st.text_input("Expression", value="3 + 4 * 2 / (1 - 5) ^ 2")

use_local_c = False
# If running locally and you have compiled C binary in project root, set use_local_c True in code or place binary in PATH.
if st.checkbox("Try local C binary (only works on your machine if compiled)"):
    # detect binary presence (./expr_eval or expr_eval.exe)
    if shutil.which("./expr_eval") or shutil.which("expr_eval"):
        use_local_c = True
    else:
        st.warning("No local C binary found (make sure you compiled C/expr_eval.c and it's in this folder).")

if st.button("Evaluate"):
    if expr.strip() == "":
        st.error("Please enter an expression.")
    else:
        # Replace ^ with ** for Python parse
        py_expr = expr.replace("^", "**")

        # Try calling local C if requested and available
        if use_local_c:
            try:
                # call local compiled binary; ensure it's executable
                proc = subprocess.run(["./expr_eval", expr], capture_output=True, text=True, timeout=5)
                if proc.returncode == 0:
                    st.success("C result: " + proc.stdout.strip())
                    st.info("Note: deployed Streamlit Cloud cannot run your local C binary. This runs only on your machine.")
                else:
                    st.error("C binary error: " + (proc.stderr.strip() or proc.stdout.strip()))
            except Exception as e:
                st.error(f"Failed to run C binary: {e}")
        else:
            # Safe Python evaluator using ast
            allowed_ops = {
                ast.Add: op.add, ast.Sub: op.sub, ast.Mult: op.mul,
                ast.Div: op.truediv, ast.Pow: op.pow,
                ast.USub: lambda a: -a, ast.UAdd: lambda a: a
            }
            def eval_node(node):
                if isinstance(node, ast.Constant):  # python3.8+: ast.Num -> ast.Constant
                    return node.value
                if isinstance(node, ast.BinOp):
                    left = eval_node(node.left)
                    right = eval_node(node.right)
                    oper = type(node.op)
                    if oper in allowed_ops:
                        return allowed_ops[oper](left, right)
                    raise ValueError("Unsupported operator")
                if isinstance(node, ast.UnaryOp):
                    operand = eval_node(node.operand)
                    oper = type(node.op)
                    if oper in allowed_ops:
                        return allowed_ops[oper](operand)
                    raise ValueError("Unsupported unary operator")
                raise ValueError("Unsupported expression")
            try:
                node = ast.parse(py_expr, mode='eval').body
                value = eval_node(node)
                st.success(f"Result: {value}")
            except Exception as e:
                st.error(f"Evaluation failed: {e}")
