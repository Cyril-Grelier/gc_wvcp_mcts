"""
Generate a xls file containing results of given methods from csv file,
the csv file name must be in the form INSTANCE_RANDSEED.csv

example :
    for the rand seed 4 for the instance queen10_10 :
        queen10_10_4.csv

The file must contain at least 2 columns named score and time.
score is the best found score
time is the time in second that took your algorithm to find the best score
Only the last line is read so it must contain the time of the best score
not the time of the end of the run.

Gap compute the difference on the mean of two methods and give the p-value
with a ttest.
"""
import statistics
from glob import glob
import re
from typing import Dict, List, Tuple
from dataclasses import dataclass

import pandas as pd
from openpyxl import Workbook
from openpyxl.cell.cell import MergedCell
from openpyxl.styles import Alignment, Font  # , PatternFill
from openpyxl.utils import get_column_letter
from scipy import stats


def main():
    """Choose the methods, gaps and instance and create the xls file"""
    # Add method name, repertory of data and short name of the method
    lss = [
        # "tabu_col",
        # "hill_climbing",
        # "afisa_original",
        "afisa",
        "tabu_weight",
        "redls",
        "ilsts",
        # "random",
        # "constrained",
        # "deterministic",
    ]
    methods = []
    for ls in lss:
        methods.append(
            Method(
                f"{ls}",
                f"outputs/output_local_search/{ls}/",
                f"{ls}",
            )
        )
        methods.append(
            Method(
                f"mcts_{ls}",
                f"outputs/output_mcts_local_search/{ls}/",
                f"mcts_{ls}",
            )
        )

    output_file = "xlsx_files/mcts_local_search.xlsx"

    # Choose the method to compare with ttest (just need the method name and short name)
    gaps = [
        (methods[i], methods[j])
        for i in range(len(methods))
        for j in range(i + 1, len(methods))
    ]
    # Choose the instances
    with open("all_instances.txt", "r", encoding="utf8") as file:
        instances = [i[:-1] for i in file.readlines()]

    t = Table(methods=methods, instances=instances, gaps=gaps)
    t.to_xls(output_file)


@dataclass
class TimeScore:
    """Store time and score"""

    time: int
    score: int

    def __repr__(self) -> str:
        return f"t:{self.time},s:{self.score}"


@dataclass
class FinalResults:
    """Store line of best results for a method"""

    best: int
    average: float
    time: float
    nb_best: int
    nb_total: int

    def to_sheet_line(self) -> List:
        """
        Convert self to a list for the sheet,
        edit length_line and sheet_line_content if you edit this function
        """
        return [self.best, self.average, self.time, f"{self.nb_best}/{self.nb_total}"]

    @staticmethod
    def length_line():
        """Return number of columns for a method"""
        return 4

    @staticmethod
    def sheet_line_content():
        """Header of method"""
        return ["best", "avg", "time", "#"]

    def __repr__(self) -> str:
        return f"b:{self.best},a:{self.average},t:{self.time},{self.nb_best}/{self.nb_total}"


@dataclass
class Gap:
    """Store the gap and p-value between 2 methods"""

    difference: float
    p_value: float

    def to_sheet_line(self):
        """Convert self to a list for the sheet"""
        return [self.difference, self.p_value]

    @staticmethod
    def length_line():
        """Return number of columns for a gap"""
        return 2

    @staticmethod
    def sheet_line_content():
        """Header of gap"""
        return ["diff", "pvalue"]

    def __repr__(self) -> str:
        return f"d:{self.difference},p:{self.p_value}"


@dataclass
class Method:
    """Store name, repertory and short name of a method/algorithm"""

    name: str
    repertory: str
    short_name: str

    def __repr__(self) -> str:
        return self.name

    def __hash__(self) -> int:
        return hash(str(self))


@dataclass
class Instance:
    """Store the results of all given method on one instance"""

    name: str
    nb_vertices: int
    nb_edges: int
    best_known_score: int
    optimal: bool
    methods: List[Method]
    method_optimal: Dict[str, bool]
    raw_results: Dict[str, List[TimeScore]]
    final_results: Dict[str, FinalResults]
    gaps: Dict[Tuple[str, str], Gap]

    def compute_final_results(self) -> None:
        """Compute results from raw data"""
        for method in self.methods:
            data: List[TimeScore] = self.raw_results[method.name]
            try:
                best_score = min([line.score for line in data])
            except TypeError:
                best_score = 999999
            except Exception as e:
                print("error :", self.name, method.__dict__)
                raise e
            self.final_results[method.name] = FinalResults(
                best=best_score,
                average=round(statistics.mean([line.score for line in data]), 1)
                if best_score != 999999
                else 999999,
                time=round(
                    statistics.mean(
                        [line.time for line in data if line.score == best_score]
                    ),
                    1,
                )
                if best_score != 999999
                else 999999,
                nb_best=len([True for line in data if line.score == best_score])
                if best_score != 999999
                else 0,
                nb_total=len(data) if best_score != 999999 else 0,
            )

    def compute_gap(self, methods_names: Tuple[Method, Method]):
        """Compute gap"""
        try:
            s1 = [line.score for line in self.raw_results[methods_names[0].name]]
        except TypeError:
            s1 = []
        try:
            s2 = [line.score for line in self.raw_results[methods_names[1].name]]
        except TypeError:
            s2 = []
        _, p_value = stats.ttest_ind(s1, s2)
        self.gaps[methods_names] = Gap(
            # difference=round(statistics.mean(s1) - statistics.mean(s2), 2),
            difference=round(
                self.final_results[methods_names[0].name].average
                - self.final_results[methods_names[1].name].average,
                1,
            ),
            p_value=round(p_value, 3),
        )

    def to_sheet_line(self):
        """
        Convert self to a list for the sheet,
        edit length_line and sheet_line_content if you edit this function
        """
        line = [
            self.name,
            self.nb_vertices,
            # self.nb_edges,
            self.best_known_score,
            self.optimal,
        ]
        for method in self.methods:
            line.extend(self.final_results[method.name].to_sheet_line())
        for _, gap in self.gaps.items():
            line.extend(gap.to_sheet_line())
        return line

    @staticmethod
    def length_line():
        """Return number of columns for a instance"""
        return 4

    @staticmethod
    def sheet_line_content():
        """Header of instance"""
        # return ["instance", "|V|", "|E|", "BKS", "optim"]
        return ["instance", "|V|", "BKS", "optim"]


def get_nb_vertices_edges(instance: str) -> Tuple[int, int]:
    with open("instances/instance_info.txt", "r", encoding="utf8") as file:
        for line in file.readlines():
            instance_, nb_vertices, nb_edges = line[:-1].split(",")
            if instance_ == instance:
                return nb_vertices, nb_edges
    raise Exception(f"instance {instance} not found in instances/instance_info.txt")


def get_best_known_score(instance: str) -> Tuple[int, bool]:
    with open("instances/best_scores_wvcp.txt", "r", encoding="utf8") as file:
        for line in file.readlines():
            instance_, score, optimal = line[:-1].split(" ")
            if instance_ == instance:
                return score, optimal == "*"
    raise Exception(f"instance {instance} not found in instances/instance_info.txt")


def load_instance(instance: str, methods: List[Method]) -> Instance:
    nb_vertices, nb_edges = get_nb_vertices_edges(instance)
    best_known_score, optimal = get_best_known_score(instance)
    return Instance(
        name=instance,
        nb_vertices=nb_vertices,
        nb_edges=nb_edges,
        best_known_score=best_known_score,
        optimal=optimal,
        methods=methods,
        method_optimal={},
        raw_results={},
        final_results={},
        gaps={},
    )


class Table:
    def __init__(
        self,
        methods: List[Method],
        instances: List[str],
        gaps: List[Tuple[Method, Method]],
    ) -> None:
        self.methods: List[Method] = methods
        self.instances: List[Instance] = [
            load_instance(instance, methods) for instance in instances
        ]
        self.gaps: List[Tuple[Method, Method]] = gaps
        for instance in self.instances:
            for method in self.methods:
                # assert (
                #     len(glob(f"{method.repertory}{instance.name}_[0-9]*.csv")) != 0
                # ), f"files not found in {method.repertory}\nInstance : {instance.name}" # TODO check
                # get the last line of the file and convert time and score to int
                instance.raw_results[method.name] = [
                    TimeScore(
                        *map(
                            int,
                            pd.read_csv(file_name)[["time", "score"]]
                            .iloc[-1]
                            .to_list(),
                        )
                    )
                    for file_name in sorted(
                        glob(f"{method.repertory}{instance.name}_[0-9]*.csv"),
                        key=lambda f: int(re.sub("\D", "", f)),
                    )
                ]
                instance.method_optimal[method.name] = False
                if not instance.raw_results[method.name]:
                    instance.raw_results[method.name] = None
                # for mcts only, check for optimality
                try:
                    for file_name in sorted(
                        glob(f"{method.repertory}{instance.name}_[0-9]*.csv"),
                        key=lambda f: int(re.sub("\D", "", f)),
                    ):
                        nb_total_node, nb_current_node = (
                            pd.read_csv(file_name)[
                                [
                                    "nb total node",
                                    "nb current node",
                                ]
                            ]
                            .iloc[-1]
                            .to_list()
                        )
                        if nb_total_node > 1 and nb_current_node <= 1:
                            instance.method_optimal[method.name] = True
                except KeyError:
                    pass

        for instance in self.instances:
            instance.compute_final_results()
        for instance in self.instances:
            for gap in self.gaps:
                instance.compute_gap(gap)

    def __repr__(self) -> str:
        return "\n".join([str(instance) for instance in self.instances])

    def to_xls(self, file_name: str):
        workbook = Workbook()
        sheet = workbook.active
        # first row
        header1 = Instance.sheet_line_content()
        firsts_cols = Instance.length_line()
        nb_col_met = FinalResults.length_line()
        nb_col_gap = Gap.length_line()

        for method in self.methods:
            header1.extend([method.name] * nb_col_met)
        for gap in self.gaps:
            header1.extend([f"gap {gap[0].short_name}-{gap[1].short_name}", ""])
        sheet.append(header1)
        # second row
        header2 = [""] * firsts_cols
        for method in self.methods:
            header2.extend(FinalResults.sheet_line_content())
        for _ in self.gaps:
            header2.extend(Gap.sheet_line_content())
        sheet.append(header2)

        # merge
        for i in range(firsts_cols):
            sheet.merge_cells(
                start_row=1, end_row=2, start_column=i + 1, end_column=i + 1
            )
        for i in range(len(self.methods)):
            sheet.merge_cells(
                start_row=1,
                end_row=1,
                start_column=firsts_cols + 1 + nb_col_met * i,
                end_column=firsts_cols + nb_col_met * (i + 1),
            )
        start_gap = firsts_cols + nb_col_met * len(self.methods) + 1
        for i in range(len(self.gaps)):
            sheet.merge_cells(
                start_row=1,
                end_row=1,
                start_column=start_gap + nb_col_gap * i,
                end_column=start_gap + nb_col_gap * i + 1,
            )

        # add data
        for instance in self.instances:
            sheet.append(instance.to_sheet_line())

        # add bold to best scores
        nb_best = {method.name: 0 for method in self.methods}
        nb_optimal = {method.name: 0 for method in self.methods}
        for row, instance in enumerate(self.instances, start=3):
            bks = instance.best_known_score
            for col, method in enumerate(self.methods):
                cell = sheet.cell(row, firsts_cols + 1 + nb_col_met * col)
                if int(cell.value) == int(bks):
                    # red for method == best know score
                    nb_best[method.name] += 1
                    cell.font = Font(bold=True, color="ffFF0000")
                elif int(cell.value) < int(bks):
                    # green for method better than best know score
                    nb_best[method.name] += 1
                    cell.font = Font(bold=True, color="ff00FF00")
                if instance.method_optimal[method.name]:
                    # blue for proven optimal score
                    nb_optimal[method.name] += 1
                    # cell.fill = PatternFill(bgColor="FFC7CE", fill_type="solid")
                    cell.font = Font(bold=True, color="ff0000ff")

        # footer
        last_line = [""] * firsts_cols
        last_line[0] = "nb total best"
        for i, method in enumerate(self.methods):
            last_line.extend(
                [f"{nb_best[method.name]}/{len(self.instances)}"] * nb_col_met
            )
        nb_positive_gap = {gap: 0 for gap in self.gaps}
        nb_negative_gap = {gap: 0 for gap in self.gaps}
        for instance in self.instances:
            for gap_name, gap in instance.gaps.items():
                if gap.difference < 0 and gap.p_value <= 0.001:  # p-value
                    nb_negative_gap[gap_name] += 1
                if gap.difference > 0 and gap.p_value <= 0.001:
                    nb_positive_gap[gap_name] += 1
        for gap in self.gaps:
            last_line.extend([f"{gap[1].short_name} better", nb_positive_gap[gap]])

        sheet.append(last_line)
        sheet.merge_cells(
            start_row=len(self.instances) + 3,
            end_row=len(self.instances) + 3,
            start_column=1,
            end_column=firsts_cols,
        )

        for i, _ in enumerate(self.methods):
            sheet.merge_cells(
                start_row=len(self.instances) + 3,
                end_row=len(self.instances) + 3,
                start_column=firsts_cols + 1 + nb_col_met * i,
                end_column=firsts_cols + nb_col_met * (i + 1),
            )

        last_line = [""] * firsts_cols
        last_line[0] = "nb total optim"
        for i, method in enumerate(self.methods):
            last_line.extend(
                [f"{nb_optimal[method.name]}/{len(self.instances)}"] * nb_col_met
            )
        for gap in self.gaps:
            last_line.extend([f"{gap[0].short_name} better", nb_negative_gap[gap]])
        sheet.append(last_line)
        sheet.merge_cells(
            start_row=len(self.instances) + 4,
            end_row=len(self.instances) + 4,
            start_column=1,
            end_column=firsts_cols,
        )

        for i, _ in enumerate(self.methods):
            sheet.merge_cells(
                start_row=len(self.instances) + 4,
                end_row=len(self.instances) + 4,
                start_column=firsts_cols + 1 + nb_col_met * i,
                end_column=firsts_cols + nb_col_met * (i + 1),
            )

        nb_positive = {method: 0 for method in self.methods}
        nb_negative = {method: 0 for method in self.methods}

        for method1_2, value in nb_positive_gap.items():
            method1, method2 = method1_2
            nb_positive[method2] += value
            nb_negative[method1] += value

        for method1_2, value in nb_negative_gap.items():
            method1, method2 = method1_2
            nb_positive[method1] += value
            nb_negative[method2] += value

        last_line = [""] * firsts_cols
        last_line[0] = "nb times significantly better"
        for i, method in enumerate(self.methods):
            last_line.extend([f"{nb_positive[method]}"] * nb_col_met)
        sheet.append(last_line)
        sheet.merge_cells(
            start_row=len(self.instances) + 5,
            end_row=len(self.instances) + 5,
            start_column=1,
            end_column=firsts_cols,
        )
        for i, _ in enumerate(self.methods):
            sheet.merge_cells(
                start_row=len(self.instances) + 5,
                end_row=len(self.instances) + 5,
                start_column=firsts_cols + 1 + nb_col_met * i,
                end_column=firsts_cols + nb_col_met * (i + 1),
            )

        last_line = [""] * firsts_cols
        last_line[0] = "nb times significantly worse"
        for i, method in enumerate(self.methods):
            last_line.extend([f"{nb_negative[method]}"] * nb_col_met)
        sheet.append(last_line)
        sheet.merge_cells(
            start_row=len(self.instances) + 6,
            end_row=len(self.instances) + 6,
            start_column=1,
            end_column=firsts_cols,
        )
        for i, _ in enumerate(self.methods):
            sheet.merge_cells(
                start_row=len(self.instances) + 6,
                end_row=len(self.instances) + 6,
                start_column=firsts_cols + 1 + nb_col_met * i,
                end_column=firsts_cols + nb_col_met * (i + 1),
            )

        last_line = [""] * firsts_cols
        last_line[0] = "ratio better/worse"
        for i, method in enumerate(self.methods):
            try:
                nb = f"{nb_positive[method] / nb_negative[method]:.2f}"
            except ZeroDivisionError:
                nb = "-"
            last_line.extend([nb] * nb_col_met)

        sheet.append(last_line)
        sheet.merge_cells(
            start_row=len(self.instances) + 7,
            end_row=len(self.instances) + 7,
            start_column=1,
            end_column=firsts_cols,
        )
        for i, _ in enumerate(self.methods):
            sheet.merge_cells(
                start_row=len(self.instances) + 7,
                end_row=len(self.instances) + 7,
                start_column=firsts_cols + 1 + nb_col_met * i,
                end_column=firsts_cols + nb_col_met * (i + 1),
            )

        # Set alignment
        for col in sheet.columns:
            for cell in col:
                cell.alignment = Alignment(horizontal="center", vertical="center")

        # Set optimal width
        column_widths = []
        for row in sheet:
            for i, cell in enumerate(row):
                if len(column_widths) > i:
                    if len(str(cell.value)) + 1 > column_widths[i]:
                        column_widths[i] = len(str(cell.value)) + 1
                else:
                    column_widths += [0]
        for i, column_width in enumerate(column_widths, 1):  # ,1 to start at 1
            sheet.column_dimensions[get_column_letter(i)].width = column_width

        # Freeze row and columns
        sheet.freeze_panes = sheet["E3"]

        # Save
        workbook.save(file_name)


if __name__ == "__main__":
    main()
