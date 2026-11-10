// Store sorting states globally
let sortState = {};
// All different radioButtons
const cycHighest = document.getElementById('cyc-highest');
const cycLowest = document.getElementById('cyc-lowest');
const cogHighest = document.getElementById('cog-highest');
const cogLowest = document.getElementById('cog-lowest');
const commitsLargest = document.getElementById('commits-largest');
const commitsSmallest = document.getElementById('commits-smallest');
const locLargest = document.getElementById('loc-largest');
const locSmallest = document.getElementById('loc-smallest');
const ascendingColor = '#AECCE4'
const descendingColor = '#77DD77'



// Function to sort the table
function sortTable(column) {
    const table = document.getElementById('file-table');
    const tbody = table.querySelector('tbody');
    const rows = Array.from(tbody.querySelectorAll('tr'));

    // Determine sort order
    if (!sortState[column]) {
        sortState[column] = 'asc'; // Default to ascending
    } else {
        sortState[column] = sortState[column] === 'asc' ? 'desc' : 'asc';
    }

    const sortOrder = sortState[column];

    // Define column index mapping
    const columnIndex = {
        'File Name': 0,
        'Size': 1,
        'Cyclomatic Complexity': 2,
        'Cognitive Complexity': 3,
        'LoC': 4,
        'Number of Commits': 5
    };

    const index = columnIndex[column];

    // Sort rows
    if(index === 0){
        rows.sort((a, b) => {
            const aText = a.children[index].innerText;
            const bText = b.children[index].innerText;

            if (aText < bText) return sortOrder === 'asc' ? -1 : 1;
            if (aText > bText) return sortOrder === 'asc' ? 1 : -1;
            return 0;
        });
    }else{
        rows.sort((a, b) => {
            const aText = a.children[index].innerText;
            const bText = b.children[index].innerText;

            const aValue = isNaN(aText) ? -1 : parseFloat(aText);
            const bValue = isNaN(bText) ? -1 : parseFloat(bText);

            if (aValue < bValue) return sortOrder === 'asc' ? -1 : 1;
            if (aValue > bValue) return sortOrder === 'asc' ? 1 : -1;
            return 0;
        });
    }


    // Clear and repopulate tbody with sorted rows
    tbody.innerHTML = '';
    rows.forEach(row => tbody.appendChild(row));
}

// Fetch and display file data in the table
async function fetchAndDisplayFileData() {
    try {
        const response = await fetch('http://127.0.0.1:5000/file_data_tables');
        if (!response.ok) throw new Error(`HTTP Error! Status: ${response.status}`);

        const data = await response.json();
        console.log("Data Retrieved from Backend:", data); // Debugging log

        const tableBody = document.querySelector('#file-table tbody');
        const totals = { size: 0, cyclomatic: 0, cognitive: 0, loc: 0 ,commits: 0 };

        tableBody.innerHTML = ''; // Clear existing rows

        if (!data.result || data.result.length === 0) {
            console.warn("No files found or result is empty.");
            tableBody.innerHTML = '<tr><td colspan="5">No files available</td></tr>';
            return;
        }

        data.result.forEach((file) => {
            // Convert "N/A" or non-numeric values to 0 for numeric fields
            const filesize = Number(file.filesize) || 0;
            const cyclomatic = file.cyclomatic_complexity === "N/A" ? 0 : Number(file.cyclomatic_complexity) || 0;
            const cognitive = file.cognitive_complexity === "N/A" ? 0 : Number(file.cognitive_complexity) || 0;
            const loc = Number(file.lines_of_code) || 0;
            const commits = Number(file.num_commits) || 0;
            const row = document.createElement('tr');
            //Show N/A anyway if theres no data for complexity while counting it as zero for the totals
            row.innerHTML = `
                <td>${file.filename}</td>
                <td>${filesize}</td>
                <td>${file.cyclomatic_complexity === "N/A" ? "N/A" : cyclomatic}</td>
                <td>${file.cognitive_complexity === "N/A" ? "N/A" : cognitive}</td>
                <td>${loc}</td>
                <td>${commits}</td>
            `;
            tableBody.appendChild(row);

            // Update totals
            totals.size += filesize;
            totals.cyclomatic += cyclomatic;
            totals.cognitive += cognitive;
            totals.loc += loc;
            totals.commits += commits;
        });

        // Update totals row
        document.getElementById('total-size').textContent = totals.size;
        document.getElementById('total-cyclomatic').textContent = totals.cyclomatic;
        document.getElementById('total-cognitive').textContent = totals.cognitive;
        document.getElementById('total-loc').textContent = totals.loc;
        document.getElementById('total-commits').textContent = totals.commits;
    } catch (error) {
        console.error('Error fetching or displaying file data:', error);
    }
}

// Load data when the page is ready
document.addEventListener('DOMContentLoaded', fetchAndDisplayFileData);

// Function to sort the table based on the selected radio option
function sortFiles() {
    const table = document.getElementById('file-table');
    const tbody = table.querySelector('tbody');
    const rows = Array.from(tbody.querySelectorAll('tr'));

    // Get the selected sorting option
    const selectedOption = document.querySelector('input[name="sort"]:checked');
    if (!selectedOption) return;

    const value = selectedOption.value; // "high" or "low"
    const id = selectedOption.id;

    resetColumnHeaderColors();

    // Determine which column to sort by
    let columnIndex = -1;

    if (id.includes('cyc')) columnIndex = 2; // Cyclomatic Complexity
    else if (id.includes('cog')) columnIndex = 3; // Cognitive Complexity
    else if (id.includes('commits')) columnIndex = 5; // Number of Commits
    else if (id.includes('loc')) columnIndex = 4; // Lines of Code

    if (columnIndex === -1) return;

    // Sort rows
    if(columnIndex === 0){
        rows.sort((a, b) => {
            const aText = a.children[columnIndex].innerText;
            const bText = b.children[columnIndex].innerText;

            if (aText < bText) return value === 'low' ? -1 : 1;
            if (aText > bText) return value === 'low' ? 1 : -1;
            return 0;
        });
    }else{
        rows.sort((a, b) => {
            const aText = a.children[columnIndex].innerText;
            const bText = b.children[columnIndex].innerText;

            const aValue = isNaN(aText) ? -1 : parseFloat(aText);
            const bValue = isNaN(bText) ? -1 : parseFloat(bText);

            if (aValue < bValue) return value === 'low' ? -1 : 1;
            if (aValue > bValue) return value === 'low' ? 1 : -1;
            return 0;
        });
    }

    // Clear and repopulate tbody with sorted rows
    tbody.innerHTML = '';
    rows.forEach(row => tbody.appendChild(row));

    updateColumnHeaderColor(columnIndex, value === 'low' ? descendingColor : ascendingColor);
}

function cleanTitlesBg(){
    document.getElementById('cycCompCol').style.background = '#2c2c2c';
    document.getElementById('cogCompCol').style.background = '#2c2c2c';
    document.getElementById('commitCol').style.background = '#2c2c2c';
    document.getElementById('locCol').style.background = '#2c2c2c';
}

// Attach event listeners to all radio buttons
document.querySelectorAll('input[name="sort"]').forEach(radio => {
    radio.addEventListener('change', sortFiles);
});

cycHighest.addEventListener('click', function(event) {
    cleanTitlesBg();
    document.getElementById('cycCompCol').style.background = ascendingColor;
});

cycLowest.addEventListener('click', function(event) {
    cleanTitlesBg();
    document.getElementById('cycCompCol').style.background = descendingColor;
})

cogLowest.addEventListener('click', function(event) {
    cleanTitlesBg();
    document.getElementById('cogCompCol').style.background = descendingColor;
})

cogHighest.addEventListener('click', function(event) {
    cleanTitlesBg();
    document.getElementById('cogCompCol').style.background = ascendingColor;
})

commitsLargest.addEventListener('click', function(event) {
    cleanTitlesBg();
    document.getElementById('commitCol').style.background = ascendingColor;
})

commitsSmallest.addEventListener('click', function(event) {
    cleanTitlesBg();
    document.getElementById('commitCol').style.background = descendingColor;
})

locLargest.addEventListener('click', function(event) {
    cleanTitlesBg();
    document.getElementById('locCol').style.background = ascendingColor;
})

locSmallest.addEventListener('click', function(event) {
    cleanTitlesBg();
    document.getElementById('locCol').style.background = descendingColor;
})

// changes the color to ascending/descending when clicked on the table
document.querySelectorAll('#file-table thead th').forEach((header, index) => {
    header.addEventListener('click', () => sortTableByColumn(index));
});

function sortTableByColumn(columnIndex) {
    const table = document.getElementById('file-table');
    const tbody = table.querySelector('tbody');
    const rows = Array.from(tbody.querySelectorAll('tr'));

    if (!sortState[columnIndex]) {
        sortState[columnIndex] = 'asc';
    } else {
        sortState[columnIndex] = sortState[columnIndex] === 'asc' ? 'desc' : 'asc';
    }

    const sortOrder = sortState[columnIndex];

    rows.sort((a, b) => {
        const aText = a.children[columnIndex].innerText;
        const bText = b.children[columnIndex].innerText;

        const aValue = isNaN(aText) ? aText : parseFloat(aText);
        const bValue = isNaN(bText) ? bText : parseFloat(bText);

        if (aValue < bValue) return sortOrder === 'asc' ? -1 : 1;
        if (aValue > bValue) return sortOrder === 'asc' ? 1 : -1;
        return 0;
    });

    tbody.innerHTML = '';
    rows.forEach(row => tbody.appendChild(row));
    resetColumnHeaderColors();
    updateColumnHeaderColor(columnIndex, sortOrder === 'asc' ? ascendingColor : descendingColor);
}

function resetColumnHeaderColors() {
    const columns = document.querySelectorAll('#file-table thead th');
    columns.forEach(header => {
        header.style.backgroundColor = '';
    });
}

function updateColumnHeaderColor(columnIndex, color) {
    const header = document.querySelector(`#file-table thead th:nth-child(${columnIndex + 1})`);
    header.style.backgroundColor = color;
}