//Attributes
// Andre
    // main file data
let data = undefined;
    // Dimensions for the SVG
const width = document.getElementById("bubble-chart").offsetWidth;
const height = document.getElementById("bubble-chart").offsetHeight;
const margin = { top: 20, right: 30, bottom: 40, left: 50 };
    // Create the SVG container
const svg = d3.select("#bubble-chart")
    .append("svg")
    .attr("width", width)
    .attr("height", height);
    // Cyclomatic buttons and constants
const cycloCompButton = document.getElementById("cycloComp");
const cogCompButton = document.getElementById("cogComp");
var currentComplexity = "cogComp";
    // Current x and y variables
var currentXAxis = "filesize";
var currentYAxis = currentComplexity;
    // X selectable buttons
const xNloc = document.getElementById("xNloc");
const xFileSize = document.getElementById("xFilesize");
const xComplexity = document.getElementById("xComplexity");
    // Y selectable buttons
const yNloc = document.getElementById("yNloc");
const yFileSize = document.getElementById("yFilesize");
const yComplexity = document.getElementById("yComplexity");
    // All variables related to path
const pathTextField = document.getElementById("path-box");
let isNormalGraph = true;
const changeScatterButton = document.getElementById("toggleGraph");
//Mariana

//Diogo Henriques
let isTrendlineVisible = false;
//Diogo

//Jose Pedro

//Alvaro
    //tooltip setup
    const tooltip = d3.select("#bubble-chart")
    .append("div")
    .attr("class", "tooltip");

//All functions
//Andre
    //Main drawing function
function drawScatter() {
    isNormalGraph = true;
    changeScatterButton.style.display = "none";
    d3.select("svg").selectAll("*").remove();
    // Create scales for x and y axes
    const x = d3.scaleLinear()
        .domain([0, d3.max(data, d => d[currentXAxis])]) // Map the "filesize" values
        .range([margin.left, width - margin.right]);

    const y = d3.scaleLinear()
        .domain([0, d3.max(data, d => d[currentYAxis])]) // Map the "cogComp" values
        .range([height - margin.bottom, margin.top]);

    // Add x-axis
    svg.append("g")
        .attr("transform", `translate(0, ${height - margin.bottom})`)
        .call(d3.axisBottom(x))
        .selectAll("text, line, path")
        .attr("stroke", "#FFF");

    // Add y-axis
    svg.append("g")
        .attr("transform", `translate(${margin.left}, 0)`)
        .call(d3.axisLeft(y))
        .selectAll("text, line, path")
        .attr("stroke", "#FFF");

    // Add dots
    svg.append('g')
        .selectAll("dot")
        .data(data) // Bind transformed data
        .enter()
        .append("circle") // Add a circle for each data point
        .attr("cx", d => x(d[currentXAxis])) // X position based on "filesize"
        .attr("cy", d => y(d[currentYAxis])) // Y position based on "cogComp"
        .attr("r", 2) // Radius of the dots
        .style("fill", "#69b3a2"); // Dot color

    //Alvaro only addHoverEffects
        // Add hover functionality to dots 
        addHoverEffects(x, y);
        addClickEffects(x, y); // Add click effect on dots
    // Add trendline
        if(isTrendlineVisible){
            addTrendline(x, y, data.map(d => d[currentXAxis]), data.map(d => d[currentYAxis]));
        }

}

    //Change cyclomatic function
function setComplexity(isCyclomatic){
    cogCompButton.checked = !isCyclomatic;
    cycloCompButton.checked = isCyclomatic;
    if (isCyclomatic === true) {
        currentComplexity = "cycComp";
    }else{
        currentComplexity = "cogComp";
    }
    // updating complexity in X or Y axis if needed
    if (currentXAxis === "cycComp" || currentXAxis === "cogComp") {
        currentXAxis = currentComplexity;
    }
    if (currentYAxis === "cycComp" || currentYAxis === "cogComp") {
        currentYAxis = currentComplexity;
    }
}

function getFileInfoByVariables(xRealValue, yRealValue) {
    // Getting the file from two variables
    for (let i in data){
        if(Math.round(data[i][currentXAxis]) === Math.round(xRealValue) && Math.round(data[i][currentYAxis]) === Math.round(yRealValue)){
            return data[i];
        }
    }
}

function drawScatterOverTime(){
    d3.json("http://127.0.0.1:5000/file_history", function(dataDict) {
        let path  = pathTextField.value
        let fileInfo = dataDict.result[path];
        if (!fileInfo){
            alert("The inserted path does not lead to a file")
            return;
        }

        isNormalGraph = false;
        changeScatterButton.style.display = "block";

        let complexity = "";
        if(currentComplexity === "cogComp"){
            complexity = "cogCompHist";
        }else{
            complexity = "cycCompHist";
        }
        let allComplexities = fileInfo[complexity].map(d => Number(d) || 0);

        d3.select("svg").selectAll("*").remove();
        // Create scales for x and y axes
        const x = d3.scaleLinear()
            .domain([0, allComplexities.length]) // Map the "filesize" values
            .range([margin.left, width - margin.right]);

        const y = d3.scaleLinear()
            .domain([0, d3.max(allComplexities)]) // Map the "cogComp" values
            .range([height - margin.bottom, margin.top]);

        const xAxis = d3.scalePoint()
            .domain(fileInfo["commitDate"]) // Pass the custom string values here
            .range([margin.left, width - margin.right]);

        // Add x-axis
        svg.append("g")
            .attr("transform", `translate(0, ${height - margin.bottom})`)
            .call(d3.axisBottom(xAxis))
            .selectAll("text, line, path")
            .attr("stroke", "#FFF");

        // Add y-axis
        svg.append("g")
            .attr("transform", `translate(${margin.left}, 0)`)
            .call(d3.axisLeft(y))
            .selectAll("text, line, path")
            .attr("stroke", "#FFF");

        // Add dots
        svg.append('g')
            .selectAll("dot")
            .data(fileInfo[complexity]) // Bind data to dots
            .enter()
            .append("circle")
            .attr("cx", (d, index) => x(index)) // Correct x based on index
            .attr("cy", d => {
                const yValue = Number(d) || 0;
                // console.log("Y Value: ", yValue); used for debugging
                return y(yValue);
            })
            .attr("r", 2)
            .style("fill", "#69b3a2");

        if (isTrendlineVisible){
            var allXNumbers = [];

            for (var i = 1; i <= allComplexities.length; i++) {
                allXNumbers.push(i);
            }
            addTrendline(x, y, allXNumbers, allComplexities);
        }
    })
}

//botões x-axis e y-axis abrem e fecham
function toggleOptions(id) {
    const options = document.getElementById(id);
    if (options.classList.contains('hidden')) {
        options.classList.remove('hidden');
        options.style.display = 'block';
    } else {
        options.classList.add('hidden');
        options.style.display = 'none';
    }
}

//Mariana

//Diogo Henriques
function addTrendline(x, y, xValues, yValues) {
    // const xValues = data.map(d => d[currentXAxis]);
    // const yValues = data.map(d => d[currentYAxis]);
    const n = xValues.length;
    const xMean = d3.mean(xValues);
    const yMean = d3.mean(yValues);

    const slope = d3.sum(xValues.map((xi, i) => (xi - xMean) * (yValues[i] - yMean))) /
                  d3.sum(xValues.map(xi => (xi - xMean) ** 2));
    const intercept = yMean - slope * xMean;

    // Define the start and end points of the trendline
    const trendlineData = [
        { x: d3.min(xValues), y: slope * d3.min(xValues) + intercept },
        { x: d3.max(xValues), y: slope * d3.max(xValues) + intercept }
    ];

    // Add trendline
    svg.append("line")
        .attr("x1", x(trendlineData[0].x))
        .attr("y1", y(trendlineData[0].y))
        .attr("x2", x(trendlineData[1].x))
        .attr("y2", y(trendlineData[1].y))
        .attr("stroke", "red")
        .attr("stroke-width", 1);
}

function toggleTrendline() {
    isTrendlineVisible = !isTrendlineVisible;

    const button = document.getElementById("toggleTrendline");
    button.textContent = isTrendlineVisible ? "Hide Trendline" : "Show Trendline";

    if(isNormalGraph){
        drawScatter();
    }else{
        drawScatterOverTime();
    }// Redraw the scatter plot with the updated state
}

function addClickEffects(x, y) {
    svg.selectAll("circle")
        .on("click", function (event, d) {
            const dot = d3.select(this); // Get the clicked dot
            const [xPos, yPos] = [parseFloat(dot.attr("cx")), parseFloat(dot.attr("cy"))];

            // Transform chart coordinates to screen coordinates
            const svgElement = document.querySelector("svg");
            const point = svgElement.createSVGPoint();
            point.x = xPos;
            point.y = yPos;
            const screenCoordinates = point.matrixTransform(svgElement.getScreenCTM());

            // Fetch file information
            const xRealValue = x.invert(xPos);
            const yRealValue = y.invert(yPos);
            const file = getFileInfoByVariables(xRealValue, yRealValue);
            // Fetch history from the backend
            fetch(`http://127.0.0.1:5000/file_history`)
            .then(response => {
                if (!response.ok) {
                    throw new Error(`HTTP error! status: ${response.status}`);
                }
                return response.json();
            })
            .then(historyData => {
                const fileHistory = historyData.result[file.filePath]; // Ensure file.filePath is correct
                if (!fileHistory) {
                    console.error("No history found for the selected file.");
                    return;
                }
             // Calculate box position and adjust for screen boundaries
                const boxWidth = 300; // Approximate width of the history box
                const boxHeight = 200; // Approximate height of the history box
                const screenWidth = window.innerWidth;
                const screenHeight = window.innerHeight;   
  
                let left = screenCoordinates.x + 10; // Offset by 10px
                let top = screenCoordinates.y + 10;  // Offset by 10px

                    // Adjust if the box would go outside the viewport
                if (left + boxWidth > screenWidth) {
                    left = screenWidth - boxWidth - 10; // Align to the right
                }
                if (top + boxHeight > screenHeight) {
                     top = screenHeight - boxHeight - 10; // Align to the bottom
                }        
                // Display the history in the box
                d3.select("#complexity-history-box")
                    .style("display", "block") // Make the box visible
                    .style("color", "black")   // Set text color to black
                    .style("left", `${left}px`)
                    .style("top", `${top}px`)
                    .html(`
                        <h3>Complexity History for ${file.filename}</h3>
                        <p><strong>Dates:</strong> ${fileHistory.commitDate.join(", ")}</p>
                        <p><strong>Cognitive Complexity:</strong> ${fileHistory.cogCompHist.join(", ")}</p>
                        <p><strong>Cyclomatic Complexity:</strong> ${fileHistory.cycCompHist.join(", ")}</p>
                        <p><strong>File Size:</strong> ${fileHistory.fileSizeHist.join(", ")}</p>
                    `);
                    console.log("History box displayed!");
            })
            .catch(err => console.error("Error fetching history:", err));
        });
}


//Diogo

//Jose Pedro

//Alvaro
// Function to add hover effects
function addHoverEffects(x ,y) {
    svg.selectAll("circle")
        .on("mouseover", function (event, d) {
            const [xPos, yPos] = [parseFloat(d3.select(this).attr("cx")), parseFloat(d3.select(this).attr("cy"))];

            // Andre -> Only get the information to the tooltip
            let xRealValue = x.invert(xPos);
            let yRealValue = y.invert(yPos);
            let file = getFileInfoByVariables(xRealValue, yRealValue);
            tooltip.html(`
                <strong>File:</strong> ${file.filename}<br>
                <strong>Cognitive complexity:</strong> ${file.cogComp}<br>
                <strong>Cyclomatic complexity:</strong> ${file.cycComp}<br>
                <strong>File Size:</strong> ${file.filesize} bytes<br>
                <strong>Author:</strong> ${file.author}
            `)
            .style("visibility", "visible")
            .style("left", `${xPos + margin.left + 250}px`) // Adjusted position ZOOM must be 100%
            .style("top", `${yPos + margin.top - 20}px`);
            d3.select(this)
                .style("fill", "red")
                .attr("r", 5); // Highlight hovered dot
        })
        .on("mouseout", function () {
            tooltip.style("visibility", "hidden");
            d3.select(this)
                .style("fill", "#69b3a2")
                .attr("r", 2); // Reset dot size
        });
}

// All events listeners

// All listeners from Andre
// event listener at start of the document
document.addEventListener("DOMContentLoaded", function () {
    //setting the correct x and y buttons
    xFileSize.checked = true;
    yComplexity.checked = true;
    // Getting all the information for the first time
    d3.json("http://127.0.0.1:5000/file_names", function(dataDict) {
        dataDict = dataDict.result;
        data = Object.entries(dataDict)
            .map(([key, d]) => ({
                filePath: key,
                filename: d.filename,
                filesize: +d.filesize || 0, // Use 0 if null or undefined
                nloc: +d.nloc || 0,
                cycComp : d.cycComp === 'N/A' || d.cycComp == null ? 0 : +d.cycComp, // Convert to number, handle 'N/A' and null
                cogComp: d.cogComp === 'N/A' || d.cogComp == null ? 0 : +d.cogComp, // Convert to number, handle 'N/A' and null
                author: d.author
            }));
        drawScatter(); // types of values chosen for the first time
    })
    // setting the complexity button
    setComplexity(false);
});

cycloCompButton.addEventListener("click", function (event) {
    setComplexity(true);
    if(isNormalGraph){
        drawScatter();
    }else{
        drawScatterOverTime()
    }

})

cogCompButton.addEventListener("click", function (event) {
    setComplexity(false);
    if(isNormalGraph){
        drawScatter();
    }else{
        drawScatterOverTime()
    }
})

xNloc.addEventListener("click", function (event) {
    currentXAxis = "nloc";
    drawScatter();
})

xComplexity.addEventListener("click", function (event) {
    currentXAxis = String(currentComplexity);
    drawScatter();
})

xFileSize.addEventListener("click", function (event) {
    currentXAxis = "filesize";
    drawScatter();
})

yNloc.addEventListener("click", function (event) {
    currentYAxis = "nloc";
    drawScatter();
})

yComplexity.addEventListener("click", function (event) {
    currentYAxis = String(currentComplexity);
    drawScatter();
})

yFileSize.addEventListener("click", function (event) {
    currentYAxis = "filesize";
    drawScatter();
})

changeScatterButton.addEventListener("click", function (event) {
    drawScatter();
})

// event listeners from mariana


// event listeners from Diogo Henriques
document.addEventListener("click", function (event) {
    const historyBox = document.getElementById("complexity-history-box");
    if (!historyBox.contains(event.target)) {
        d3.select("#complexity-history-box").style("display", "none");
    }
});
document.addEventListener("DOMContentLoaded", () => {
    const button = document.getElementById("toggleTrendline");
    if (button) {
        button.addEventListener("click", toggleTrendline);
    }
});
// event listeners from Diogo


// event listeners from Jose Pedro


// event listeners from Álvaro
