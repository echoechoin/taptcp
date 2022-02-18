from rich.console import Console
from rich.table import Table
import sys


table_row_len = 0

if (sys.argv[1]):
    try:
        table_row_len = int(sys.argv[1])
    except:
        print("expect argument type: int")
        exit()
data_count = len(sys.argv) - 2
if table_row_len * 2 != data_count:
    print("expect argument count:", table_row_len * 2)
    exit()
console = Console()

table = Table(show_header=True, header_style="bold magenta")
for i in range(0, table_row_len):
    table.add_column(str(sys.argv[i+2]), justify="right")

table.add_row(
    *sys.argv[table_row_len + 2:]
)


console.print(table)