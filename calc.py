#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
المشروع: الحاسبة المتقدمة
الوصف: برنامج حاسبة متقدمة  يدعم العمليات الحسابية الأساسية والعلمية والرسم البياني،
حل المعادلات، العمليات التفاضلية والتكاملية، الإحصاء، حساب المصفوفات، والتحويلات الوحدات.
"""

# ===================== المكتبات =====================
import tkinter as tk
from tkinter import ttk, messagebox
import math
import sympy as sp
import matplotlib
matplotlib.use("TkAgg")
from matplotlib.backends.backend_tkagg import FigureCanvasTkAgg
from matplotlib.figure import Figure
import statistics
import sys

# ===================== إدارة التاريخ والذاكرة =====================
class HistoryManager:
    """
    فئة لإدارة تاريخ العمليات الحسابية.
    تخزن العمليات كأزواج (تعبير الحساب, النتيجة).
    """
    def __init__(self):
        self.history = []  # قائمة لتخزين العمليات الحسابية السابقة

    def add_entry(self, expression, result):
        """إضافة سجل جديد إلى التاريخ."""
        self.history.append((expression, result))

    def clear_history(self):
        """مسح التاريخ كاملًا."""
        self.history = []

    def get_history(self):
        """إرجاع قائمة التاريخ الحالية."""
        return self.history

class MemoryManager:
    """
    فئة لإدارة الذاكرة في الآلة الحاسبة.
    تدعم العمليات الجمع والطرح والاسترجاع.
    """
    def __init__(self):
        self.memory = 0.0  # قيمة مبدئية للذاكرة

    def add(self, value):
        """إضافة قيمة إلى الذاكرة."""
        self.memory += value

    def subtract(self, value):
        """طرح قيمة من الذاكرة."""
        self.memory -= value

    def recall(self):
        """إرجاع قيمة الذاكرة الحالية."""
        return self.memory

    def clear(self):
        """مسح الذاكرة وإعادة تعيينها إلى 0."""
        self.memory = 0.0

# ===================== الآلة الحاسبة الأساسية =====================
class BasicCalculator(tk.Frame):
    """
    الآلة الحاسبة الأساسية:
    تدعم العمليات الحسابية البسيطة (الجمع والطرح والضرب والقسمة) مع دعم للذاكرة.
    """
    def __init__(self, parent, history_manager, memory_manager, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.history_manager = history_manager
        self.memory_manager = memory_manager
        self.create_widgets()

    def create_widgets(self):
        # إعداد المتغيرات لعرض التعبير والنتيجة
        self.entry_var = tk.StringVar()
        self.result_var = tk.StringVar()

        # مربع الإدخال الرئيسي
        self.entry = tk.Entry(self, textvariable=self.entry_var, font=("Arial", 20),
                              bd=4, relief=tk.RIDGE, justify="right")
        self.entry.grid(row=0, column=0, columnspan=4, padx=10, pady=10, sticky="nsew")

        # إعداد أزرار العمليات الأساسية مع شبكة التخطيط
        for i in range(6):
            self.grid_rowconfigure(i, weight=1)
        for j in range(4):
            self.grid_columnconfigure(j, weight=1)

        # تعريف مجموعة أزرار الآلة الحاسبة الأساسية
        button_texts = [
            "7", "8", "9", "/",
            "4", "5", "6", "*",
            "1", "2", "3", "-",
            "0", ".", "=", "+",
            "C", "MC", "M+", "M-"
        ]
        row = 1
        col = 0
        for bt in button_texts:
            action = lambda x=bt: self.on_button_click(x)
            btn = tk.Button(self, text=bt, font=("Arial", 18), command=action)
            btn.grid(row=row, column=col, sticky="nsew", padx=5, pady=5)
            col += 1
            if col > 3:
                col = 0
                row += 1

        # عرض النتيجة في ملصق (يمكن توسيعه لاحقًا)
        self.result_label = tk.Label(self, textvariable=self.result_var, font=("Arial", 20), bg="lightgray")
        self.result_label.grid(row=row, column=0, columnspan=4, padx=10, pady=10, sticky="nsew")

    def on_button_click(self, char):
        """
        معالجة نقرات الأزرار:
        - "C": مسح التعبير والنتيجة.
        - "=": تقييم التعبير.
        - "MC", "M+", "M-": التعامل مع الذاكرة.
        - الأزرار الأخرى تُضاف إلى التعبير.
        """
        if char == "C":
            self.entry_var.set("")
            self.result_var.set("")
        elif char == "=":
            expression = self.entry_var.get()
            try:
                # السماح باستخدام دوال الرياضيات من مكتبة math فقط
                allowed = {k: getattr(math, k) for k in dir(math) if not k.startswith("__")}
                allowed["sqrt"] = math.sqrt
                result = eval(expression, {"__builtins__": None}, allowed)
                self.result_var.set(str(result))
                self.history_manager.add_entry(expression, result)
            except Exception as e:
                self.result_var.set("خطأ")
        elif char == "MC":
            self.memory_manager.clear()
        elif char == "M+":
            try:
                value = float(self.result_var.get())
                self.memory_manager.add(value)
            except Exception:
                pass
        elif char == "M-":
            try:
                value = float(self.result_var.get())
                self.memory_manager.subtract(value)
            except Exception:
                pass
        else:
            current = self.entry_var.get()
            self.entry_var.set(current + char)

# ===================== الآلة الحاسبة العلمية =====================
class ScientificCalculator(tk.Frame):
    """
    الآلة الحاسبة العلمية:
    تدعم التعبيرات الرياضية المعقدة والدوال العلمية مثل sin, cos, tan, log, ln وغيرها.
    """
    def __init__(self, parent, history_manager, memory_manager, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.history_manager = history_manager
        self.memory_manager = memory_manager
        self.create_widgets()

    def create_widgets(self):
        self.expr_var = tk.StringVar()
        self.res_var = tk.StringVar()

        # مربع الإدخال للتعبير الرياضي
        self.entry = tk.Entry(self, textvariable=self.expr_var, font=("Arial", 20),
                              bd=4, relief=tk.RIDGE, justify="right")
        self.entry.grid(row=0, column=0, columnspan=5, padx=10, pady=10, sticky="nsew")

        # إعداد شبكة التخطيط
        for i in range(8):
            self.grid_rowconfigure(i, weight=1)
        for j in range(5):
            self.grid_columnconfigure(j, weight=1)

        # تعريف أزرار العلوم
        sci_buttons = [
            "sin", "cos", "tan", "log", "ln",
            "asin", "acos", "atan", "sqrt", "^",
            "(", ")", "pi", "e", "factorial",
            "exp", "abs", "round", "floor", "ceil",
            "C", "=", "MC", "M+", "M-"
        ]
        row = 1
        col = 0
        for bt in sci_buttons:
            action = lambda x=bt: self.on_button_click(x)
            btn = tk.Button(self, text=bt, font=("Arial", 16), command=action)
            btn.grid(row=row, column=col, sticky="nsew", padx=3, pady=3)
            col += 1
            if col >= 5:
                col = 0
                row += 1

    def on_button_click(self, char):
        """معالجة أزرار الآلة الحاسبة العلمية."""
        if char == "C":
            self.expr_var.set("")
            self.res_var.set("")
        elif char == "=":
            expr = self.expr_var.get()
            try:
                # إعداد قاموس آمن يحتوي على دوال الرياضيات
                safe_dict = {k: getattr(math, k) for k in dir(math) if not k.startswith("__")}
                safe_dict.update({"pi": math.pi, "e": math.e, "factorial": math.factorial})
                # تحويل علامة ^ إلى ** للتربيع وما شابه
                expr = expr.replace("^", "**")
                result = eval(expr, {"__builtins__": None}, safe_dict)
                self.res_var.set(str(result))
                self.history_manager.add_entry(expr, result)
            except Exception as e:
                self.res_var.set("خطأ")
        elif char == "MC":
            self.memory_manager.clear()
        elif char == "M+":
            try:
                value = float(self.res_var.get())
                self.memory_manager.add(value)
            except Exception:
                pass
        elif char == "M-":
            try:
                value = float(self.res_var.get())
                self.memory_manager.subtract(value)
            except Exception:
                pass
        else:
            current = self.expr_var.get()
            self.expr_var.set(current + char)

# ===================== الآلة الحاسبة الرسومية =====================
class GraphingCalculator(tk.Frame):
    """
    الآلة الحاسبة الرسومية:
    تعرض رسم بياني لدالة رياضية يدخلها المستخدم.
    """
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.create_widgets()

    def create_widgets(self):
        self.function_var = tk.StringVar()

        # تعليمات الإدخال للمستخدم (يستخدم متغير x)
        label = tk.Label(self, text="ادخل الدالة (باستخدام المتغير x):", font=("Arial", 16))
        label.pack(pady=5)

        self.entry = tk.Entry(self, textvariable=self.function_var, font=("Arial", 16), width=30)
        self.entry.pack(pady=5)

        plot_button = tk.Button(self, text="ارسم الدالة", font=("Arial", 16), command=self.plot_function)
        plot_button.pack(pady=5)

        # إعداد الرسم البياني باستخدام matplotlib
        self.fig = Figure(figsize=(5, 4), dpi=100)
        self.ax = self.fig.add_subplot(111)
        self.canvas = FigureCanvasTkAgg(self.fig, master=self)
        self.canvas.get_tk_widget().pack(fill=tk.BOTH, expand=True)

    def plot_function(self):
        """تعالج عملية رسم الدالة البيانية باستخدام sympy."""
        func_str = self.function_var.get()
        self.ax.clear()
        try:
            x = sp.symbols('x')
            expr = sp.sympify(func_str)
            f = sp.lambdify(x, expr, "math")
            xs = [i/10.0 for i in range(-100, 101)]
            ys = [f(val) for val in xs]
            self.ax.plot(xs, ys, label=func_str)
            self.ax.legend()
            self.canvas.draw()
        except Exception as e:
            self.ax.text(0.5, 0.5, "خطأ في الدالة", horizontalalignment='center',
                         verticalalignment='center', transform=self.ax.transAxes)
            self.canvas.draw()

# ===================== حل المعادلات =====================
class EquationSolver(tk.Frame):
    """
    حل المعادلات الجبرية باستخدام sympy.
    يجب أن تحتوي المعادلة على علامة "=" للفصل بين طرفي المعادلة.
    """
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.create_widgets()

    def create_widgets(self):
        instructions = tk.Label(self, text="ادخل المعادلة (يجب استخدام '=' للفصل):", font=("Arial", 16))
        instructions.pack(pady=5)

        self.eq_var = tk.StringVar()
        self.entry = tk.Entry(self, textvariable=self.eq_var, font=("Arial", 16), width=40)
        self.entry.pack(pady=5)

        solve_button = tk.Button(self, text="حل المعادلة", font=("Arial", 16), command=self.solve_equation)
        solve_button.pack(pady=5)

        self.result_text = tk.Text(self, font=("Arial", 14), height=10)
        self.result_text.pack(pady=5, fill=tk.BOTH, expand=True)

    def solve_equation(self):
        """حل المعادلة وعرض الحلول في مربع النص."""
        eq_str = self.eq_var.get()
        self.result_text.delete("1.0", tk.END)
        try:
            if "=" not in eq_str:
                self.result_text.insert(tk.END, "صيغة المعادلة غير صحيحة؛ يجب أن تحتوي على '='.\n")
                return
            left, right = eq_str.split("=")
            x = sp.symbols('x')
            left_expr = sp.sympify(left)
            right_expr = sp.sympify(right)
            solutions = sp.solve(left_expr - right_expr, x)
            self.result_text.insert(tk.END, "الحلول:\n")
            for sol in solutions:
                self.result_text.insert(tk.END, str(sol) + "\n")
        except Exception as e:
            self.result_text.insert(tk.END, "خطأ في حل المعادلة.\n")

# ===================== العمليات التفاضلية والتكاملية (حساب التفاضل والتكامل) =====================
class CalculusCalculator(tk.Frame):
    """
    تبويب للعمليات التفاضلية والتكاملية:
    يمكن حساب المشتقة، والتكامل (المحدد أو غير المحدد)، والنهايات.
    """
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        # تعريف الرمز المستخدم (x) في التعابير الرياضية
        self.x = sp.symbols('x')
        self.create_widgets()

    def create_widgets(self):
        # عنوان الإدخال
        title = tk.Label(self, text="أدخل التعبير (بالاعتماد على المتغير x):", font=("Arial", 16))
        title.grid(row=0, column=0, columnspan=4, padx=10, pady=10, sticky="w")

        self.expr_entry = tk.Entry(self, font=("Arial", 16))
        self.expr_entry.grid(row=1, column=0, columnspan=4, padx=10, pady=5, sticky="ew")

        # قسم المشتقة
        der_label = tk.Label(self, text="حساب المشتقة", font=("Arial", 14, "bold"))
        der_label.grid(row=2, column=0, padx=10, pady=5, sticky="w")
        der_button = tk.Button(self, text="احسب المشتقة", font=("Arial", 14), command=self.compute_derivative)
        der_button.grid(row=2, column=1, padx=10, pady=5)

        # قسم التكامل مع إمكانية تحديد حدود التكامل
        int_label = tk.Label(self, text="حساب التكامل:", font=("Arial", 14, "bold"))
        int_label.grid(row=3, column=0, padx=10, pady=5, sticky="w")
        low_label = tk.Label(self, text="الحد الأدنى:", font=("Arial", 12))
        low_label.grid(row=3, column=1, padx=5, pady=5, sticky="e")
        self.low_entry = tk.Entry(self, font=("Arial", 12), width=8)
        self.low_entry.grid(row=3, column=2, padx=5, pady=5, sticky="w")
        high_label = tk.Label(self, text="الحد الأعلى:", font=("Arial", 12))
        high_label.grid(row=3, column=3, padx=5, pady=5, sticky="e")
        self.high_entry = tk.Entry(self, font=("Arial", 12), width=8)
        self.high_entry.grid(row=3, column=4, padx=5, pady=5, sticky="w")
        int_button = tk.Button(self, text="احسب التكامل", font=("Arial", 14), command=self.compute_integral)
        int_button.grid(row=4, column=1, padx=10, pady=5)

        # قسم النهايات
        lim_label = tk.Label(self, text="حساب النهاية عند النقطة:", font=("Arial", 14, "bold"))
        lim_label.grid(row=5, column=0, padx=10, pady=5, sticky="w")
        self.lim_entry = tk.Entry(self, font=("Arial", 14), width=10)
        self.lim_entry.grid(row=5, column=1, padx=5, pady=5, sticky="w")
        lim_button = tk.Button(self, text="احسب النهاية", font=("Arial", 14), command=self.compute_limit)
        lim_button.grid(row=5, column=2, padx=10, pady=5)

        # مربع عرض النتائج
        self.calc_result = tk.Text(self, font=("Arial", 14), height=8)
        self.calc_result.grid(row=6, column=0, columnspan=5, padx=10, pady=10, sticky="nsew")

        for i in range(7):
            self.grid_rowconfigure(i, weight=1)
        for j in range(5):
            self.grid_columnconfigure(j, weight=1)

    def compute_derivative(self):
        """حساب مشتقة التعبير الرياضي."""
        expr_str = self.expr_entry.get()
        try:
            expr = sp.sympify(expr_str)
            derivative = sp.diff(expr, self.x)
            self.calc_result.insert(tk.END, f"مشتقة {expr_str} بالنسبة لـ x:\n{derivative}\n\n")
        except Exception as e:
            self.calc_result.insert(tk.END, "خطأ في حساب المشتقة.\n\n")

    def compute_integral(self):
        """حساب التكامل المحدد أو غير المحدد."""
        expr_str = self.expr_entry.get()
        try:
            expr = sp.sympify(expr_str)
            low = self.low_entry.get().strip()
            high = self.high_entry.get().strip()
            if low and high:
                # التكامل المحدد
                lower = sp.sympify(low)
                upper = sp.sympify(high)
                integral = sp.integrate(expr, (self.x, lower, upper))
                self.calc_result.insert(tk.END,
                                        f"التكامل المحدد لـ {expr_str} من {low} إلى {high}:\n{integral}\n\n")
            else:
                # التكامل غير المحدد
                integral = sp.integrate(expr, self.x)
                self.calc_result.insert(tk.END, f"التكامل غير المحدد لـ {expr_str}:\n{integral}\n\n")
        except Exception as e:
            self.calc_result.insert(tk.END, "خطأ في حساب التكامل.\n\n")

    def compute_limit(self):
        """حساب النهاية عند نقطة معينة."""
        expr_str = self.expr_entry.get()
        point = self.lim_entry.get().strip()
        try:
            expr = sp.sympify(expr_str)
            if point:
                pt = sp.sympify(point)
                limit_value = sp.limit(expr, self.x, pt)
                self.calc_result.insert(tk.END,
                                        f"نهاية {expr_str} عند x = {point}:\n{limit_value}\n\n")
            else:
                self.calc_result.insert(tk.END, "الرجاء إدخال النقطة لحساب النهاية.\n\n")
        except Exception as e:
            self.calc_result.insert(tk.END, "خطأ في حساب النهاية.\n\n")

# ===================== الحسابات الإحصائية =====================
class StatisticsCalculator(tk.Frame):
    """
    تبويب للحسابات الإحصائية:
    يتم إدخال مجموعة من القيم (يفصل بينها بفاصلة) ويحسب البرنامج المتوسط، الوسيط،
    الانحراف المعياري، والتباين.
    """
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.create_widgets()

    def create_widgets(self):
        # تعليمات الإدخال
        label = tk.Label(self, text="أدخل الأرقام مفصولة بفواصل:", font=("Arial", 16))
        label.pack(pady=10)

        self.numbers_entry = tk.Entry(self, font=("Arial", 16), width=50)
        self.numbers_entry.pack(pady=5)

        # زر لحساب الإحصائيات
        calc_button = tk.Button(self, text="احسب الإحصائيات", font=("Arial", 16), command=self.calculate_stats)
        calc_button.pack(pady=10)

        # مربع عرض النتائج
        self.stats_text = tk.Text(self, font=("Arial", 14), height=10)
        self.stats_text.pack(padx=10, pady=10, fill=tk.BOTH, expand=True)

    def calculate_stats(self):
        """معالجة الحسابات الإحصائية وعرض النتائج."""
        nums_str = self.numbers_entry.get()
        try:
            # تحويل السلسلة إلى قائمة من الأرقام
            numbers = [float(num.strip()) for num in nums_str.split(",") if num.strip() != ""]
            if not numbers:
                raise ValueError("لا توجد أرقام صالحة.")
            mean_val = statistics.mean(numbers)
            median_val = statistics.median(numbers)
            stdev_val = statistics.stdev(numbers) if len(numbers) > 1 else 0.0
            variance_val = statistics.variance(numbers) if len(numbers) > 1 else 0.0

            # عرض النتائج
            self.stats_text.delete("1.0", tk.END)
            self.stats_text.insert(tk.END, f"المتوسط الحسابي: {mean_val}\n")
            self.stats_text.insert(tk.END, f"الوسيط: {median_val}\n")
            self.stats_text.insert(tk.END, f"الانحراف المعياري: {stdev_val}\n")
            self.stats_text.insert(tk.END, f"التباين: {variance_val}\n")
        except Exception as e:
            self.stats_text.delete("1.0", tk.END)
            self.stats_text.insert(tk.END, "خطأ في معالجة الأرقام المدخلة.\n")

# ===================== عمليات المصفوفات =====================
class MatrixCalculator(tk.Frame):
    """
    تبويب لعمليات المصفوفات:
    يدعم عمليات الجمع، الطرح، الضرب، حساب المحدد، والمعكوس.
    يجب إدخال المصفوفات بطريقة صفوف مفصولة بأسطر، والعناصر بفواصل.
    """
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.operation_options = ["جمع", "طرح", "ضرب", "محدد", "معكوس"]
        self.selected_op = tk.StringVar(value=self.operation_options[0])
        self.create_widgets()

    def create_widgets(self):
        # تعليمات عامة للمستخدم
        inst_label = tk.Label(self, text="أدخل المصفوفة A (صفوف بفاصل أسطر والعناصر بفواصل):", font=("Arial", 14))
        inst_label.grid(row=0, column=0, padx=10, pady=5, sticky="w")
        self.matrix_a = tk.Text(self, font=("Arial", 14), height=6, width=40)
        self.matrix_a.grid(row=1, column=0, padx=10, pady=5)

        # للمسائل التي تحتاج لمصفوفة ثانية (جمع، طرح، ضرب)
        inst_label_b = tk.Label(self, text="أدخل المصفوفة B (إن لزم الأمر):", font=("Arial", 14))
        inst_label_b.grid(row=0, column=1, padx=10, pady=5, sticky="w")
        self.matrix_b = tk.Text(self, font=("Arial", 14), height=6, width=40)
        self.matrix_b.grid(row=1, column=1, padx=10, pady=5)

        # قائمة اختيار العملية
        op_label = tk.Label(self, text="اختر العملية:", font=("Arial", 14))
        op_label.grid(row=2, column=0, padx=10, pady=5, sticky="w")
        op_menu = tk.OptionMenu(self, self.selected_op, *self.operation_options)
        op_menu.config(font=("Arial", 14))
        op_menu.grid(row=2, column=1, padx=10, pady=5, sticky="w")

        # زر لتنفيذ العملية
        calc_button = tk.Button(self, text="نفذ العملية", font=("Arial", 16), command=self.compute_matrix)
        calc_button.grid(row=3, column=0, columnspan=2, padx=10, pady=10)

        # مربع عرض النتائج
        self.matrix_result = tk.Text(self, font=("Arial", 14), height=8)
        self.matrix_result.grid(row=4, column=0, columnspan=2, padx=10, pady=10, sticky="nsew")

        self.grid_rowconfigure(4, weight=1)
        self.grid_columnconfigure(0, weight=1)
        self.grid_columnconfigure(1, weight=1)

    def parse_matrix(self, text):
        """تحويل النص إلى مصفوفة باستخدام sympy.Matrix."""
        try:
            rows = [row.strip() for row in text.strip().split("\n") if row.strip() != ""]
            matrix_list = []
            for r in rows:
                # فصل القيم بحسب الفاصلة أو الفراغ
                if "," in r:
                    elements = [float(num.strip()) for num in r.split(",") if num.strip() != ""]
                else:
                    elements = [float(num.strip()) for num in r.split() if num.strip() != ""]
                matrix_list.append(elements)
            return sp.Matrix(matrix_list)
        except Exception as e:
            raise ValueError("خطأ في تحليل المصفوفة.")

    def compute_matrix(self):
        """تنفيذ العملية المحددة على المصفوفات المدخلة."""
        op = self.selected_op.get()
        try:
            A_text = self.matrix_a.get("1.0", tk.END)
            A = self.parse_matrix(A_text)
        except Exception as e:
            self.matrix_result.delete("1.0", tk.END)
            self.matrix_result.insert(tk.END, "خطأ في قراءة المصفوفة A.\n")
            return

        result = None
        try:
            if op in ["جمع", "طرح", "ضرب"]:
                # يجب قراءة المصفوفة الثانية
                B_text = self.matrix_b.get("1.0", tk.END)
                B = self.parse_matrix(B_text)
                if op == "جمع":
                    result = A + B
                elif op == "طرح":
                    result = A - B
                elif op == "ضرب":
                    result = A * B
            elif op == "محدد":
                result = A.det()
            elif op == "معكوس":
                result = A.inv() if A.det() != 0 else "المصفوفة غير قابلة للعكس"
            # عرض النتيجة
            self.matrix_result.delete("1.0", tk.END)
            self.matrix_result.insert(tk.END, f"النتيجة:\n{result}\n")
        except Exception as e:
            self.matrix_result.delete("1.0", tk.END)
            self.matrix_result.insert(tk.END, "حدث خطأ أثناء عملية المصفوفة.\n")

# ===================== محوِّل الوحدات =====================
class UnitConverter(tk.Frame):
    """
    تبويب محوّل الوحدات:
    يدعم تحويل الوحدات في ثلاثة مجالات: الطول، الوزن، ودرجة الحرارة.
    """
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        # الفئات المتاحة للتحويل
        self.categories = ["الطول", "الوزن", "درجة الحرارة"]
        self.selected_category = tk.StringVar(value=self.categories[0])
        # الوحدات المتاحة لكل فئة
        self.units = {
            "الطول": ["متر", "كيلومتر", "ميل", "سنتيمتر"],
            "الوزن": ["كيلوغرام", "جرام", "باوند"],
            "درجة الحرارة": ["سيلسيوس", "فهرنهايت", "كلفن"]
        }
        self.from_unit = tk.StringVar(value=self.units[self.categories[0]][0])
        self.to_unit = tk.StringVar(value=self.units[self.categories[0]][1])
        self.create_widgets()

    def create_widgets(self):
        # اختيار الفئة
        cat_label = tk.Label(self, text="اختر الفئة:", font=("Arial", 14))
        cat_label.grid(row=0, column=0, padx=10, pady=5, sticky="w")
        cat_menu = tk.OptionMenu(self, self.selected_category, *self.categories, command=self.update_units)
        cat_menu.config(font=("Arial", 14))
        cat_menu.grid(row=0, column=1, padx=10, pady=5, sticky="w")

        # إدخال القيمة للتحويل
        val_label = tk.Label(self, text="أدخل القيمة:", font=("Arial", 14))
        val_label.grid(row=1, column=0, padx=10, pady=5, sticky="w")
        self.value_entry = tk.Entry(self, font=("Arial", 14))
        self.value_entry.grid(row=1, column=1, padx=10, pady=5, sticky="w")

        # قائمة الوحدات المصدر
        from_label = tk.Label(self, text="من وحدة:", font=("Arial", 14))
        from_label.grid(row=2, column=0, padx=10, pady=5, sticky="w")
        self.from_menu = tk.OptionMenu(self, self.from_unit, *self.units[self.selected_category.get()])
        self.from_menu.config(font=("Arial", 14))
        self.from_menu.grid(row=2, column=1, padx=10, pady=5, sticky="w")

        # قائمة الوحدة الهدف
        to_label = tk.Label(self, text="إلى وحدة:", font=("Arial", 14))
        to_label.grid(row=3, column=0, padx=10, pady=5, sticky="w")
        self.to_menu = tk.OptionMenu(self, self.to_unit, *self.units[self.selected_category.get()])
        self.to_menu.config(font=("Arial", 14))
        self.to_menu.grid(row=3, column=1, padx=10, pady=5, sticky="w")

        # زر التحويل
        conv_button = tk.Button(self, text="حول الوحدة", font=("Arial", 16), command=self.convert_unit)
        conv_button.grid(row=4, column=0, columnspan=2, padx=10, pady=10)

        # عرض النتيجة
        self.result_label = tk.Label(self, text="", font=("Arial", 16), bg="lightyellow")
        self.result_label.grid(row=5, column=0, columnspan=2, padx=10, pady=10, sticky="nsew")

        for i in range(6):
            self.grid_rowconfigure(i, weight=1)
        for j in range(2):
            self.grid_columnconfigure(j, weight=1)

    def update_units(self, event=None):
        """تحديث القوائم المنسدلة للوحدات عند تغيير الفئة."""
        cat = self.selected_category.get()
        unit_list = self.units[cat]
        self.from_unit.set(unit_list[0])
        self.to_unit.set(unit_list[1] if len(unit_list) > 1 else unit_list[0])
        # إعادة بناء قوائم الوحدات
        self.from_menu['menu'].delete(0, 'end')
        self.to_menu['menu'].delete(0, 'end')
        for unit in unit_list:
            self.from_menu['menu'].add_command(label=unit, command=tk._setit(self.from_unit, unit))
            self.to_menu['menu'].add_command(label=unit, command=tk._setit(self.to_unit, unit))

    def convert_unit(self):
        """تنفيذ تحويل الوحدة مع الأخذ بالاعتبار الفئة المختارة."""
        try:
            value = float(self.value_entry.get())
            cat = self.selected_category.get()
            from_u = self.from_unit.get()
            to_u = self.to_unit.get()
            result = None

            if cat == "الطول":
                # تحويل الوحدات بين متر، كيلومتر، ميل، وسنتيمتر
                factors = {
                    "متر": 1.0,
                    "كيلومتر": 1000.0,
                    "ميل": 1609.34,
                    "سنتيمتر": 0.01
                }
                # التحويل يتم عبر القيمة بالوحدة الأساسية (المتر)
                result = value * factors[from_u] / factors[to_u]
            elif cat == "الوزن":
                # تحويل الوحدات بين كيلوغرام، جرام، وباوند
                factors = {
                    "كيلوغرام": 1.0,
                    "جرام": 0.001,
                    "باوند": 0.453592
                }
                result = value * factors[from_u] / factors[to_u]
            elif cat == "درجة الحرارة":
                # تحويل درجات الحرارة
                if from_u == "سيلسيوس" and to_u == "فهرنهايت":
                    result = (value * 9/5) + 32
                elif from_u == "فهرنهايت" and to_u == "سيلسيوس":
                    result = (value - 32) * 5/9
                elif from_u == "سيلسيوس" and to_u == "كلفن":
                    result = value + 273.15
                elif from_u == "كلفن" and to_u == "سيلسيوس":
                    result = value - 273.15
                elif from_u == "فهرنهايت" and to_u == "كلفن":
                    result = (value - 32) * 5/9 + 273.15
                elif from_u == "كلفن" and to_u == "فهرنهايت":
                    result = (value - 273.15) * 9/5 + 32
                else:
                    result = value
            else:
                result = value
            self.result_label.config(text=f"النتيجة: {result}")
        except Exception as e:
            self.result_label.config(text="خطأ في التحويل.")

# ===================== إعدادات التطبيق =====================
class SettingsFrame(tk.Frame):
    """
    تبويب الإعدادات:
    يمكن إضافة مزايا وضبط إعدادات التطبيق لاحقًا.
    """
    def __init__(self, parent, *args, **kwargs):
        super().__init__(parent, *args, **kwargs)
        self.create_widgets()

    def create_widgets(self):
        label = tk.Label(self, text="الإعدادات قيد التطوير. سيتم إضافتها لاحقاً!", font=("Arial", 18))
        label.pack(pady=20)
        
        # مثال على زر إعادة تهيئة التاريخ والذاكرة
        reset_button = tk.Button(self, text="إعادة تعيين التاريخ والذاكرة", font=("Arial", 16), command=self.reset_all)
        reset_button.pack(pady=10)
        
        self.status_label = tk.Label(self, text="لا توجد إعدادات إضافية حالياً.", font=("Arial", 14))
        self.status_label.pack(pady=10)
    
    def reset_all(self):
        messagebox.showinfo("إعدادات", "ميزة إعادة التعيين قيد التطوير.")

# ===================== التطبيق الرئيسي =====================
class AdvancedCalculatorApp(tk.Tk):
    """
    التطبيق الرئيسي الذي يجمع جميع التبويبات في نافذة واحدة باستخدام Notebook.
    """
    def __init__(self):
        super().__init__()
        self.title("الحاسبة المتقدمة")
        self.geometry("1100x800")
        
        # تهيئة مديري التاريخ والذاكرة
        self.history_manager = HistoryManager()
        self.memory_manager = MemoryManager()
        
        # إنشاء Notebook وإضافة التبويبات المختلفة
        self.notebook = ttk.Notebook(self)
        self.notebook.pack(fill=tk.BOTH, expand=True)

        # إنشاء كافة الإطارات (التبويبات)
        self.basic_calc = BasicCalculator(self.notebook, self.history_manager, self.memory_manager)
        self.scientific_calc = ScientificCalculator(self.notebook, self.history_manager, self.memory_manager)
        self.graphing_calc = GraphingCalculator(self.notebook)
        self.equation_solver = EquationSolver(self.notebook)
        self.calculus_calc = CalculusCalculator(self.notebook)
        self.statistics_calc = StatisticsCalculator(self.notebook)
        self.matrix_calc = MatrixCalculator(self.notebook)
        self.unit_converter = UnitConverter(self.notebook)
        self.settings_frame = SettingsFrame(self.notebook)

        # إضافة التبويبات مع عناوينها
        self.notebook.add(self.basic_calc, text="أساسي")
        self.notebook.add(self.scientific_calc, text="علمي")
        self.notebook.add(self.graphing_calc, text="رسومي")
        self.notebook.add(self.equation_solver, text="حل المعادلات")
        self.notebook.add(self.calculus_calc, text="التفاضل والتكامل")
        self.notebook.add(self.statistics_calc, text="إحصائي")
        self.notebook.add(self.matrix_calc, text="مصفوفات")
        self.notebook.add(self.unit_converter, text="محول وحدات")
        self.notebook.add(self.settings_frame, text="إعدادات")
        
        # إنشاء شريط قائمة للتطبيق
        self.create_menu()
        
        # شريط الحالة لعرض آخر العمليات الحسابية (من التاريخ)
        self.status_bar = tk.Label(self, text="التاريخ فارغ", bd=1, relief=tk.SUNKEN, anchor=tk.W, font=("Arial", 12))
        self.status_bar.pack(side=tk.BOTTOM, fill=tk.X)
        
        # تحديث شريط الحالة بشكل دوري ليعرض أحدث العمليات
        self.update_history()

    def create_menu(self):
        menubar = tk.Menu(self)
        filemenu = tk.Menu(menubar, tearoff=0)
        filemenu.add_command(label="خروج", command=self.quit)
        menubar.add_cascade(label="ملف", menu=filemenu)
        self.config(menu=menubar)

    def update_history(self):
        """تحديث شريط الحالة بتاريخ العمليات الحسابية الأخيرة."""
        history_entries = self.history_manager.get_history()
        if history_entries:
            last_entries = history_entries[-5:]
            text = " | ".join([f"{expr} = {res}" for expr, res in last_entries])
        else:
            text = "التاريخ فارغ"
        self.status_bar.config(text=text)
        # تحديث كل 3 ثوانٍ
        self.after(3000, self.update_history)

# ===================== نقطة الدخول الرئيسية =====================
if __name__ == "__main__":
    try:
        app = AdvancedCalculatorApp()
        app.mainloop()
    except Exception as exc:
        # طباعة الخطأ في حال فشل تشغيل التطبيق
        print("حدث خطأ:", exc)
        sys.exit(1)
