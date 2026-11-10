#include <bits/stdc++.h>
using namespace std;

int main() {
    ios::sync_with_stdio(false);
    cin.tie(nullptr);

    int P, W;
    cin >> P >> W;

    vector<vector<pair<int,int>>> adj(P + 1);   // adj[u] = {v, w}
    vector<vector<int>> radj(P + 1);            // reverse graph (no weights needed)

    for (int i = 0; i < W; i++) {
        int u, v, w;
        cin >> u >> v >> w;
        adj[u].push_back({v, w});
        radj[v].push_back(u);
    }

    // ---- Kosaraju Phase 1: iterative DFS → finish order ----
    vector<bool> visited(P + 1, false);
    vector<int> finish_order;
    finish_order.reserve(P);

    for (int s = 1; s <= P; s++) {
        if (visited[s]) continue;
        visited[s] = true;
        // stack stores (node, edge_index)
        vector<pair<int,int>> stk;
        stk.push_back({s, 0});
        while (!stk.empty()) {
            auto& [node, ei] = stk.back();
            // skip visited neighbors
            while (ei < (int)adj[node].size() && visited[adj[node][ei].first])
                ei++;
            if (ei < (int)adj[node].size()) {
                int nxt = adj[node][ei].first;
                visited[nxt] = true;
                ei++;
                stk.push_back({nxt, 0});
            } else {
                finish_order.push_back(node);
                stk.pop_back();
            }
        }
    }

    // ---- Kosaraju Phase 2: DFS on reverse graph in reverse finish order ----
    fill(visited.begin(), visited.end(), false);
    vector<vector<int>> scc_members;

    for (int i = (int)finish_order.size() - 1; i >= 0; i--) {
        int s = finish_order[i];
        if (visited[s]) continue;
        scc_members.push_back({});
        vector<int>& members = scc_members.back();
        vector<int> stk = {s};
        visited[s] = true;
        while (!stk.empty()) {
            int node = stk.back(); stk.pop_back();
            members.push_back(node);
            for (int v : radj[node]) {
                if (!visited[v]) {
                    visited[v] = true;
                    stk.push_back(v);
                }
            }
        }
    }

    // ---- Negative cycle detection per SCC via SPFA ----
    // All nodes start at distance 0 (virtual-source trick).
    // If any node is enqueued more than n times, a negative cycle exists.
    vector<vector<int>> result;

    for (auto& members : scc_members) {
        int n = (int)members.size();

        if (n == 1) {
            int node = members[0];
            for (auto [v, w] : adj[node])
                if (v == node && w < 0) { result.push_back({node}); break; }
            continue;
        }

        // Map node → local index
        unordered_map<int,int> loc;
        loc.reserve(n * 2);
        for (int i = 0; i < n; i++) loc[members[i]] = i;

        // Build SCC subgraph
        vector<vector<pair<int,int>>> sub(n);
        for (int node : members) {
            int u = loc[node];
            for (auto [v, w] : adj[node]) {
                auto it = loc.find(v);
                if (it != loc.end())
                    sub[u].push_back({it->second, w});
            }
        }

        // SPFA
        vector<long long> dist(n, 0);
        vector<bool> in_q(n, true);
        vector<int> cnt(n, 1);
        deque<int> q;
        for (int i = 0; i < n; i++) q.push_back(i);

        bool found = false;
        while (!q.empty() && !found) {
            int u = q.front(); q.pop_front();
            in_q[u] = false;
            long long du = dist[u];
            for (auto [v, w] : sub[u]) {
                long long nd = du + w;
                if (nd < dist[v]) {
                    dist[v] = nd;
                    if (!in_q[v]) {
                        in_q[v] = true;
                        if (++cnt[v] > n) { found = true; break; }
                        q.push_back(v);
                    }
                }
            }
        }

        if (found) {
            sort(members.begin(), members.end());
            result.push_back(members);
        }
    }

    sort(result.begin(), result.end());

    cout << result.size() << '\n';
    for (auto& scc : result) {
        for (int i = 0; i < (int)scc.size(); i++) {
            if (i) cout << ' ';
            cout << scc[i];
        }
        cout << '\n';
    }

    return 0;
}
